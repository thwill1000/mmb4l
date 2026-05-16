#!/bin/bash

# Script to change commits made before 7pm on weekdays to the following Sunday.
# Checks BOTH author date and committer date independently, since they can differ
# after rebases, cherry-picks, amends, or patch application.
# Usage: ./git-date-fix.sh

set -e

BRANCH=$(git branch --show-current)

echo "Current branch: $BRANCH"
echo "Looking for commits with author OR committer date before 7pm (19:00) on weekdays (Mon-Fri)"
echo "Starting from February 1, 2025"
echo ""

# Helper: given a timestamp, return the next Sunday's date at the same time,
# or empty string if the timestamp is NOT a weekday before 19:00.
needs_rewrite() {
    local timestamp="$1"
    local day_of_week hour commit_date time_part tz_part days_to_sunday next_sunday

    day_of_week=$(date -d "$timestamp" +%u)
    hour=$(date -d "$timestamp" +%H)

    if [ "$day_of_week" -ge 1 ] && [ "$day_of_week" -le 5 ] && [ "$hour" -lt 19 ]; then
        commit_date=$(date -d "$timestamp" +%Y-%m-%d)
        time_part=$(date -d "$timestamp" +%H:%M:%S)
        tz_part=$(date -d "$timestamp" +%z)
        days_to_sunday=$((7 - day_of_week))
        next_sunday=$(date -d "$commit_date + $days_to_sunday days" +"%Y-%m-%d")
        echo "$next_sunday $time_part $tz_part"
    else
        echo ""
    fi
}

echo "Scanning commits..."
echo ""

> /tmp/commits_list.txt

git log --since="2025-02-01" --format="%H|%aI|%cI|%s" --reverse | while IFS='|' read -r hash author_ts committer_ts subject; do
    new_author_date=$(needs_rewrite "$author_ts")
    new_committer_date=$(needs_rewrite "$committer_ts")

    # Skip if neither date needs changing
    if [ -z "$new_author_date" ] && [ -z "$new_committer_date" ]; then
        continue
    fi

    # Fall back to unchanged value if only one needs rewriting
    [ -z "$new_author_date" ]    && new_author_date="$author_ts"
    [ -z "$new_committer_date" ] && new_committer_date="$committer_ts"

    author_day=$(date -d "$author_ts" +%A)
    committer_day=$(date -d "$committer_ts" +%A)

    echo "Commit: ${hash:0:8}"
    echo "  Message:             $subject"
    echo "  Author date:         $author_ts ($author_day)"
    if [ "$new_author_date" != "$author_ts" ]; then
        echo "  New author date:     $new_author_date (Sunday) ✓"
    else
        echo "  Author date:         (no change)"
    fi
    echo "  Committer date:      $committer_ts ($committer_day)"
    if [ "$new_committer_date" != "$committer_ts" ]; then
        echo "  New committer date:  $new_committer_date (Sunday) ✓"
    else
        echo "  Committer date:      (no change)"
    fi
    echo ""

    echo "$hash|$new_author_date|$new_committer_date" >> /tmp/commits_list.txt
done

if [ ! -s /tmp/commits_list.txt ]; then
    echo "No commits found that need to be changed."
    rm -f /tmp/commits_list.txt
    exit 0
fi

MATCH_COUNT=$(wc -l < /tmp/commits_list.txt)
echo "----------------------------------------"
echo "Found $MATCH_COUNT commit(s) to rewrite."
echo ""
read -p "Do you want to proceed with rewriting commit dates? (yes/no): " confirm

if [ "$confirm" != "yes" ]; then
    echo "Aborted. No changes made."
    rm -f /tmp/commits_list.txt
    exit 0
fi

echo ""
echo "Rewriting git history..."
echo ""

# Build env-filter script with separate author and committer dates per commit
cat > /tmp/date-env-filter.sh << 'EOF'
case $GIT_COMMIT in
EOF

while IFS='|' read -r hash new_author_date new_committer_date; do
    echo "    $hash)" >> /tmp/date-env-filter.sh
    echo "        export GIT_AUTHOR_DATE='$new_author_date'" >> /tmp/date-env-filter.sh
    echo "        export GIT_COMMITTER_DATE='$new_committer_date'" >> /tmp/date-env-filter.sh
    echo "        ;;" >> /tmp/date-env-filter.sh
done < /tmp/commits_list.txt

cat >> /tmp/date-env-filter.sh << 'EOF'
esac
EOF

# Find the earliest commit that needs to be changed
FIRST_COMMIT=$(head -n 1 /tmp/commits_list.txt | cut -d'|' -f1)
PARENT_COMMIT=$(git rev-parse ${FIRST_COMMIT}^ 2>/dev/null || echo "--root")

echo "First commit to change: ${FIRST_COMMIT:0:8}"
if [ "$PARENT_COMMIT" = "--root" ]; then
    echo "This is the root commit; rewriting from the start."
    RANGE="HEAD"
else
    echo "Starting rewrite from parent: ${PARENT_COMMIT:0:8}"
    RANGE="${PARENT_COMMIT}..HEAD"
fi
echo ""

export FILTER_BRANCH_SQUELCH_WARNING=1
git filter-branch -f --env-filter "$(cat /tmp/date-env-filter.sh)" -- $RANGE

# Cleanup
rm -f /tmp/commits_list.txt /tmp/date-env-filter.sh

echo ""
echo "========================================"
echo "SUCCESS! Commit dates have been updated."
echo "========================================"
echo ""
echo "⚠️  IMPORTANT: Git history has been rewritten."
echo ""
echo "Next steps:"
echo "  1. Verify the changes: git log --since='2025-02-01' --format='%h %aI %cI %s'"
echo "  2. If satisfied, force push: git push origin $BRANCH --force"
echo "  3. Team members will need to: git fetch origin && git reset --hard origin/$BRANCH"

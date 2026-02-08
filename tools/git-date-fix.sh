#!/bin/bash

# Script to change commits made before 7pm on weekdays to the following Sunday
# Usage: ./rewrite_commit_dates.sh

set -e

BRANCH=$(git branch --show-current)

echo "Current branch: $BRANCH"
echo "Looking for commits made before 7pm (19:00) on weekdays (Mon-Fri)"
echo "Starting from February 1, 2025"
echo ""

# Create a list of commits to change
echo "Scanning commits..."
echo ""

> /tmp/commits_list.txt

git log --since="2025-02-01" --format="%H|%aI|%s" --reverse | while IFS='|' read -r hash timestamp subject; do
    day_of_week=$(date -d "$timestamp" +%u)
    hour=$(date -d "$timestamp" +%H)

    if [ "$day_of_week" -ge 1 ] && [ "$day_of_week" -le 5 ] && [ "$hour" -lt 19 ]; then
        day_name=$(date -d "$timestamp" +%A)
        commit_date=$(date -d "$timestamp" +%Y-%m-%d)
        time_part=$(date -d "$timestamp" +%H:%M:%S)
        tz_part=$(date -d "$timestamp" +%z)
        days_to_sunday=$((7 - day_of_week))
        next_sunday=$(date -d "$commit_date + $days_to_sunday days" +"%Y-%m-%d")
        new_timestamp="$next_sunday $time_part $tz_part"

        echo "Commit: ${hash:0:8}"
        echo "  Message: $subject"
        echo "  Current: $timestamp ($day_name)"
        echo "  Will change to: $new_timestamp (Sunday)"
        echo ""

        echo "$hash|$new_timestamp" >> /tmp/commits_list.txt
    fi
done

if [ ! -s /tmp/commits_list.txt ]; then
    echo "No commits found that need to be changed."
    rm -f /tmp/commits_list.txt
    exit 0
fi

echo "----------------------------------------"
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

# Create the date mapping as a shell script
cat > /tmp/date-env-filter.sh << 'EOF'
case $GIT_COMMIT in
EOF

while IFS='|' read -r hash new_date; do
    echo "    $hash) export GIT_AUTHOR_DATE='$new_date'; export GIT_COMMITTER_DATE='$new_date' ;;" >> /tmp/date-env-filter.sh
done < /tmp/commits_list.txt

cat >> /tmp/date-env-filter.sh << 'EOF'
esac
EOF

# Find the earliest commit that needs to be changed
FIRST_COMMIT=$(head -n 1 /tmp/commits_list.txt | cut -d'|' -f1)

# Get the parent of the first commit to change (this will be the base)
PARENT_COMMIT=$(git rev-parse ${FIRST_COMMIT}^)

echo "First commit to change: ${FIRST_COMMIT:0:8}"
echo "Starting rewrite from parent: ${PARENT_COMMIT:0:8}"
echo ""

# Run filter-branch only from the parent of the first affected commit
export FILTER_BRANCH_SQUELCH_WARNING=1
git filter-branch -f --env-filter "$(cat /tmp/date-env-filter.sh)" -- ${PARENT_COMMIT}..HEAD

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
echo "  1. Verify the changes: git log --since='2025-02-01'"
echo "  2. If satisfied, force push: git push origin $BRANCH --force"
echo "  3. Team members will need to: git fetch origin && git reset --hard origin/$BRANCH"

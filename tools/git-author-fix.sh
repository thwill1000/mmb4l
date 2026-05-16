#!/bin/bash

# Script to rewrite commits authored/committed by Thomas Williams <thomas-williams@siemens.com>
# to Thomas Hugo Williams <thomas.hugo.williams@gmail.com>
# Usage: ./git-author-fix.sh

set -e

OLD_NAME="Thomas Williams"
OLD_EMAIL="thomas-williams@siemens.com"
NEW_NAME="Thomas Hugo Williams"
NEW_EMAIL="thomas.hugo.williams@gmail.com"

BRANCH=$(git branch --show-current)

echo "Current branch: $BRANCH"
echo "Looking for commits by: $OLD_NAME <$OLD_EMAIL>"
echo "Will replace with:      $NEW_NAME <$NEW_EMAIL>"
echo ""

# Scan for matching commits
echo "Scanning commits..."
echo ""

> /tmp/commits_to_fix.txt

git log --format="%H|%aI|%an|%ae|%cn|%ce|%s" --reverse | while IFS='|' read -r hash timestamp author_name author_email committer_name committer_email subject; do
    needs_change=false

    if [ "$author_name" = "$OLD_NAME" ] && [ "$author_email" = "$OLD_EMAIL" ]; then
        needs_change=true
    fi
    if [ "$committer_name" = "$OLD_NAME" ] && [ "$committer_email" = "$OLD_EMAIL" ]; then
        needs_change=true
    fi

    if [ "$needs_change" = true ]; then
        echo "Commit: ${hash:0:8}"
        echo "  Message:   $subject"
        echo "  Date:      $timestamp"
        echo "  Author:    $author_name <$author_email>"
        echo "  Committer: $committer_name <$committer_email>"
        echo ""

        echo "$hash" >> /tmp/commits_to_fix.txt
    fi
done

if [ ! -s /tmp/commits_to_fix.txt ]; then
    echo "No commits found matching $OLD_NAME <$OLD_EMAIL>."
    rm -f /tmp/commits_to_fix.txt
    exit 0
fi

MATCH_COUNT=$(wc -l < /tmp/commits_to_fix.txt)
echo "----------------------------------------"
echo "Found $MATCH_COUNT commit(s) to rewrite."
echo ""
read -p "Do you want to proceed with rewriting author/committer info? (yes/no): " confirm

if [ "$confirm" != "yes" ]; then
    echo "Aborted. No changes made."
    rm -f /tmp/commits_to_fix.txt
    exit 0
fi

echo ""
echo "Rewriting git history..."
echo ""

# Find the earliest commit that needs to be changed
FIRST_COMMIT=$(head -n 1 /tmp/commits_to_fix.txt)
PARENT_COMMIT=$(git rev-parse ${FIRST_COMMIT}^  2>/dev/null || echo "--root")

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
export OLD_NAME OLD_EMAIL NEW_NAME NEW_EMAIL

git filter-branch -f --env-filter '
    if [ "$GIT_AUTHOR_NAME"    = "$OLD_NAME"  ] && \
       [ "$GIT_AUTHOR_EMAIL"   = "$OLD_EMAIL" ]; then
        export GIT_AUTHOR_NAME="$NEW_NAME"
        export GIT_AUTHOR_EMAIL="$NEW_EMAIL"
    fi

    if [ "$GIT_COMMITTER_NAME"  = "$OLD_NAME"  ] && \
       [ "$GIT_COMMITTER_EMAIL" = "$OLD_EMAIL" ]; then
        export GIT_COMMITTER_NAME="$NEW_NAME"
        export GIT_COMMITTER_EMAIL="$NEW_EMAIL"
    fi
' -- $RANGE

# Cleanup
rm -f /tmp/commits_to_fix.txt

echo ""
echo "========================================"
echo "SUCCESS! Author/committer info updated."
echo "========================================"
echo ""
echo "⚠️  IMPORTANT: Git history has been rewritten."
echo ""
echo "Next steps:"
echo "  1. Verify the changes: git log --format='%h %an <%ae> %s'"
echo "  2. If satisfied, force push: git push origin $BRANCH --force"
echo "  3. Team members will need to: git fetch origin && git reset --hard origin/$BRANCH"

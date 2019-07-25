this is czf.c;

git:

0.git checkout -b czf_dev
1. git push origin czf_dev
2. vim czf_dev5.c
3. git add czf_dev5.c
4. git commit -m "add czf_dev5.c"
5. git checkout master
6. git merge czf_dev
7. git branch --set-upstream-to=origin/czf_dev
8. git push origin master:czf_dev

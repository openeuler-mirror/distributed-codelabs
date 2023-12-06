# **提交说明**

- 每人一个分支适配特定的问题，提前记录在群公告中，避免重复工作

- 提交代码时按照模块目录+修改点提交，如

  ```
  git commit -m "foundation/distributeddatamgr/relational_store: remove hisysevent"
  ```

  多次commit可以合并，暂时先不要合并，最终分支合并或者提交openEuler时再合并。

- 适配完成后，根据各个模块修改的commit生成补丁，应用到其他分支，最后合并到统一的分支。
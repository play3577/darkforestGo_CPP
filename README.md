# darkforestGo_CPP
The Go APP, reconstruction use vc++, based on darkforestGO, .
学习DarkForestGO，用CPP调用源程序写的DEMO。原来程序是用Lua+C的。
一 board模块 
官方说明：
* `./board`
Things about board and its evaluations. Board data structure and different playout policy.

笔记：

棋盘

1 检查某个位置是否可以走子 TryPlay

2 在某个位置放置棋子       Play

3 获得棋盘上所有棋子信息   GetAllStones

  （走子之后，有可能要提子，这个函数是提子之后所有棋子信息）
  
4 检查成绩 GetFastScore

源程序在VC2008和VC2010下编译通过。
这里没有带工程文件，自己用VC创建一个新工程，把源码加进来编译通过就可以测试。
目前只测试了Board模块。



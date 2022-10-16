# 文法分类器

本文法分类器使用Python3进行编写
对文法类型的判断主要集中于对产生式规则的判断
对某条化简的产生式规则判断的流程如下

1. 左侧缺终结符号?是则判定为不是Chomsky文法, 退出
2. 左侧比右侧长?是则判断为0型, 退出
3. 左侧长度大于1?是则判断为1型, 退出
4. 右侧长度大于2?是则判断为2型, 退出
5. 右侧长度等于1且是非终结符号?是则判断为2型, 退出
6. 右侧长度等于2且不符合终结符号+非终结符号?是则判断为2型, 退出
7. 判断为3型, 退出

## 0型测试

```
请输入文法:     G[N] 
请输入Vn:       N, A, B
请输入产生式规则, 结束输入请输入"exit"
请输入第1行:    N ::= AB
请输入第2行:    AB ::= 1 
请输入第3行:    A ::= ABA
请输入第4行:    exit
产生式规则输入结束
文法 G[N] = ({'B', 'A', 'N'}, {'1'}, Pro, N)
Pro:    N ::= AB
        AB ::= 1
        A ::= ABA
该文法是 Chomsky0 型文法
```

## 1型测试

```
请输入文法:     G[N] 
请输入Vn:       N, B   
请输入产生式规则, 结束输入请输入"exit"
请输入第1行:    N ::= 12B
请输入第2行:    2B ::= 456
请输入第3行:    N ::= 34B
请输入第4行:    exit
产生式规则输入结束
文法 G[N] = ({'N', 'B'}, {'1', '2', '4', '3', '6', '5'}, Pro, N)
Pro:    N ::= 12B
        2B ::= 456
        N ::= 34B
该文法是 Chomsky1 型文法
```

## 2型测试

```
请输入文法:     G[N]
请输入Vn:       N, A, B
请输入产生式规则, 结束输入请输入"exit"
请输入第1行:    N ::= AB
请输入第2行:    A ::= 12
请输入第3行:    B ::= 3B
请输入第4行:    exit
产生式规则输入结束
文法 G[N] = ({'A', 'B', 'N'}, {'2', '3', '1'}, Pro, N)
Pro:    N ::= AB
        A ::= 12
        B ::= 3B
该文法是 Chomsky2 型文法
```

## 3型测试

```
请输入文法:     G[N]
请输入Vn:       N, A, B
请输入产生式规则, 结束输入请输入"exit"
请输入第1行:    N ::= 1A
请输入第2行:    A ::= 2A|3B|4
请输入第3行:    B ::= 0
请输入第4行:    exit
产生式规则输入结束
文法 G[N] = ({'A', 'N', 'B'}, {'1', '3', '4', '0', '2'}, Pro, N)
Pro:    N ::= 1A
        A ::= 2A|3B|4
        B ::= 0
该文法是 Chomsky3 型文法
```
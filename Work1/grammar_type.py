import re

class Grammar:
    
    class StartSymbolException(Exception):
        def __init__(self):
            pass
        def __str__(self):
            return repr('开始符号不在非终结符号内, 程序退出...')
    
    def __init__(self, name, start, vn):
        self.name = str(name)
        self.start = str(start)
        self.vn = set(vn)
        self.vt = set([])
        self.type = 3
        self.pro = []
        
        if self.start not in self.vn:
            raise Grammar.StartSymbolException()
    
    def get_rules_type(self, input_str, output_str):
        '''get grammar Chomsky type'''
        input_len = len(input_str)
        output_len = len(output_str)
        
        has_vn = False
        for ch in (input_str + output_str):
            if ch not in self.vn:
                self.vt.add(ch)
            else:
                has_vn = True
        
        # 左侧缺非终结符号, 不是正确的文法
        if not has_vn:
            return -1

        # type 0
        if input_len > output_len:
            return 0
        # type 1
        if input_len > 1:
            return 1
        if input_str not in self.vn:
            return 1
        # type 2
        if output_len > 2:
            return 2
        if output_len == 1:
            if output_str in self.vn:
                return 2
        if output_len == 2:
            if output_str[0] in self.vn or \
                output_str[1] in self.vt:
                    return 2
        # type 3
        return 3
    
    def add_rules(self, input_str, output_strs):
        for output_str in output_strs:
            t = self.get_rules_type(input_str, output_str)
            if t < self.type:
                self.type = t
                
        out = '|'.join(output_strs)
        rule = ''.join([input_str, ' ::= ', out])
        self.pro.append(rule)
    
    
def parse_grammar(gram):
    st = 0
    name = ''
    start = ''
    gram = gram.strip()
    
    # 提取名称与起始符号
    for ch in gram:
        # 不允许非字母名称与起始符
        if (not ch.isalnum()) and ch != '[' and ch != ']':
            return False, '', ''
        
        if ch == '[':
            # 不合理的'['出现
            if st != 0:
                return False, '', ''
            st = 1
            continue
        if ch == ']':
            # 不合理的']'出现
            if st != 1:
                return False, '', ''
            st = 2
            continue
        
        if st == 0:
            name += ch
        elif st == 1:
            start += ch
        else:
            # ']'后依然有符号
            return False, '', ''
        
    if st != 2 or name == '' or start == '':
        return False, '', ''
    
    return True, name, start


def parse_vn(vn):
    n_strs = vn.split(',')
    n_strs = [ele.strip() for ele in n_strs]
    
    # 不是单字符或不是字母数字
    for n_str in n_strs:
        if n_str.__len__() != 1 or not n_str.isalnum():
            return False, []
        
    return True, n_strs


def parse_rule(rule):
    sp = rule.find('::=')
    input_str = rule[:sp].strip()
    output_strs = rule[sp + 3:].split('|')
    output_strs = [ele.strip() for ele in output_strs]
    
    # 不是字母数字
    if not input_str.isalnum():
        return False, '', []
    for output_str in output_strs:
        if not output_str.isalnum():
            return False, '', []
        
    return True, input_str, output_strs
    
    
def main():
    
    # 输入文法
    while True:
        gram = input('请输入文法:\t')
        _, name, start = parse_grammar(gram)
        
        if not _:
            print('格式不规范, 请重新输入, 示例: "G[N]"')
            continue
        
        break
    
    # 输入Vn
    while True:
        vn = input('请输入Vn:\t')
        _, n_strs = parse_vn(vn)
        
        if not _:
            print('格式不规范, 请重新输入, 示例: "A, B, C"')
            continue
        
        break
        
    # 出现开始符号不在非终结符号内的情况则退出
    try:
        grammar = Grammar(name, start, n_strs)
    except Grammar.StartSymbolException:
        print('开始符号不在非终结符号内, 程序退出...')
        return
    
    # 输入产生式规则
    line = 1
    print('请输入产生式规则, 结束输入请输入"exit"')
    while True:
        rule = input(f'请输入第{line}行:\t')
        
        if rule.strip() == 'exit':
            print('产生式规则输入结束')
            break
        
        _, input_str, output_strs = parse_rule(rule)
        
        if not _:
            print('格式不规范, 请重新输入, 示例: "A ::= BC|d"')
            continue
        
        grammar.add_rules(input_str, output_strs)
        line += 1
        
    # 输出结果
    print(f'文法 {grammar.name}[{grammar.start}] = ({grammar.vn}, {grammar.vt}, Pro, {grammar.start})')
    print('Pro:', end='')
    for p in grammar.pro:
        print(f'\t{p}')
    if grammar.type == -1:
        print(f'该文法左侧缺少非终结符号, 不是 Chomsky 文法')
    else:
        print(f'该文法是 Chomsky{grammar.type} 型文法')
    

if __name__ == '__main__':
    main()
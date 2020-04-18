import sys
import os
import platform


class IRSyntaxError(Exception):
    pass


class DuplicatedLabelError(Exception):
    pass


class UndefinedLabelError(Exception):
    pass


class DuplicatedVariableError(Exception):
    pass


class CurrentFunctionNoneError(Exception):
    pass


class IRSim():

    def __init__(self):
        pass

    def fileOpen(self, fname):
        if not fname:
            return -1
        self.initialize()
        fp = open(fname, 'r')
        lineno = 0
        for line in fp:
            if line.isspace():
                continue
            if self.sanity_check(line, lineno):
                self.codeList.append(line.strip().replace('\t', ' '))
            else:
                break
            lineno += 1
        else:
            self.filename = fname
            self.lineno = lineno

        fp.close()

        if self.entranceIP == -1:
            sys.stderr.write(
                "Error: Cannot find program entrance. Please make sure the 'main' function does exist.\n")
        if not self.labelCheck() or self.offset > 1048576 or self.entranceIP == -1:
            self.initialize()
            return -1
        else:
            self.mem = [0] * 262144
            return 0

    def initialize(self):
        self.filename = None
        self.ip = -1
        self.entranceIP = -1
        self.pauseRunning = False
        self.offset = 0
        self.instrCnt = 0
        self.codes = list()
        self.mem = None
        self.functionDict = dict()
        self.currentFunction = None
        self.symTable = dict()
        self.labelTable = dict()
        self.callStack = list()
        self.argumentStack = list()
        self.displayFunction = list()
        self.codeList = list()
        return

    def labelCheck(self):
        try:
            for i in range(self.lineno):
                code = unicode(self.codeList[i])
                strs = code.split()
                if strs[0] == 'GOTO':
                    if strs[1] not in self.labelTable:
                        raise UndefinedLabelError
                elif strs[0] == 'IF':
                    if strs[5] not in self.labelTable:
                        raise UndefinedLabelError
                elif len(strs) > 2 and strs[2] == 'CALL':
                    if strs[3] not in self.labelTable:
                        raise UndefinedLabelError

        except UndefinedLabelError:
            sys.stderr.write(
                'Error: Undefined label at line %d:\n%s\n' % (i + 1, code))
            return False

        return True

    def run(self):
        self.stop()
        self.ip = self.entranceIP
        while True:
            if self.ip < 0 or self.ip >= len(self.codes):
                error_code = 3
                break
            code = self.codes[self.ip]
            error_code = self.execute_code(code)
            if error_code > 0:
                break
            self.ip += 1

        ret = 0
        if error_code == 1:
            sys.stderr.write(
                'Program has exited gracefully.\nTotal instructions = %d\n' % self.instrCnt)
            ret = self.instrCnt
        elif error_code == 2:
            sys.stderr.write("Illegal memory access at %d\n" & (self.ip + 1))
            ret = -1
        elif error_code == 3:
            sys.stderr.write("Program Counter goes out of bound.\n")
            # Goes out of bound
            ret = -2
        self.ip = -1
        return ret

    def stop(self):
        self.ip = -1
        self.instrCnt = 0
        self.pauseRunning = False
        self.mem = [0] * 262144
        self.callStack = list()
        self.argumentStack = list()
        self.displayFunction = list()
        self.displayFunction.append('main')

    def step(self):
        if self.ip == -1:
            self.stop()
            self.pauseRunning = True
            self.ip = self.entranceIP - 1
        self.ip += 1
        if self.ip < 0 or self.ip >= len(self.codes):
            sys.stderr.write("Program Counter goes out of bound.\n")
            self.ip = -1
            self.pauseRunning = False
            return
        code = self.codes[self.ip]
        error_code = self.execute_code(code)
        if error_code == 1:
            sys.stderr.write(
                'Program has exited gracefully.\nTotal instructions = %d\n' % self.instrCnt)
            self.ip = -1
            self.pauseRunning = False
        elif error_code == 2:
            sys.stderr.write("Illegal memory access at %d\n" & (self.ip + 1))
            self.ip = -1
            self.pauseRunning = False

    def sanity_check(self, code, lineno):
        strs = code.split()
        relops = ['>', '<', '>=', '<=', '==', '!=']
        arithops = ['+', '-', '*', '/']
        try:
            if strs[0] == 'LABEL' or strs[0] == 'FUNCTION':
                if len(strs) != 3 or strs[2] != ':':
                    raise IRSyntaxError
                if strs[1] in self.labelTable:
                    raise DuplicatedLabelError
                self.labelTable[strs[1]] = lineno
                if strs[1] == 'main':
                    if strs[0] == 'LABEL':
                        raise IRSyntaxError
                    self.entranceIP = lineno
                if strs[0] == 'FUNCTION':
                    self.currentFunction = strs[1]
                    self.functionDict[strs[1]] = list()
                self.codes.append(('LABEL', strs[1]))
            else:
                if self.currentFunction == None:
                    raise CurrentFunctionNoneError
                if strs[0] == 'GOTO':
                    if len(strs) != 2:
                        raise IRSyntaxError
                    self.codes.append(('GOTO', strs[1]))
                elif strs[0] == 'RETURN' or strs[0] == 'READ' or strs[0] == 'WRITE' or strs[0] == 'ARG' or strs[0] == 'PARAM':
                    if len(strs) != 2:
                        raise IRSyntaxError
                    if (strs[0] == 'READ' or strs[0] == 'PARAM') and not strs[1][0].isalpha():
                        raise IRSyntaxError
                    self.tableInsert(strs[1])
                    self.codes.append((strs[0], strs[1]))
                elif strs[0] == 'DEC':
                    if len(strs) != 3 or int(strs[2]) % 4 != 0:
                        raise IRSyntaxError
                    if strs[1] in self.symTable:
                        raise DuplicatedVariableError
                    self.tableInsert(strs[1], int(strs[2]), True)
                    self.codes.append('DEC')
                elif strs[0] == 'IF':
                    if len(strs) != 6 or strs[4] != 'GOTO' or strs[2] not in relops:
                        raise IRSyntaxError
                    self.tableInsert(strs[1])
                    self.tableInsert(strs[3])
                    self.codes.append(
                        ('IF', strs[1], strs[2], strs[3], strs[5]))
                else:
                    if strs[1] != ':=' or len(strs) < 3:
                        raise IRSyntaxError
                    if strs[0][0] == '&' or strs[0][0] == '#':
                        raise IRSyntaxError
                    self.tableInsert(strs[0])
                    if strs[2] == 'CALL':
                        if len(strs) != 4:
                            raise IRSyntaxError
                        self.codes.append(('CALL', strs[0], strs[3]))
                    elif len(strs) == 3:
                        self.tableInsert(strs[2])
                        self.codes.append(('MOV', strs[0], strs[2]))
                    elif len(strs) == 5 and strs[3] in arithops:
                        self.tableInsert(strs[2])
                        self.tableInsert(strs[4])
                        self.codes.append(
                            ('ARITH', strs[0], strs[2], strs[3], strs[4]))
                    else:
                        raise IRSyntaxError
        except (IRSyntaxError, ValueError):
            sys.stderr.write(
                'Syntax error at line %d:\n%s\n' % (lineno + 1, code))
            return False
        except DuplicatedLabelError:
            sys.stderr.write('Duplicated label %s at line %d:\n%s\n' % (
                strs[1], lineno + 1, code))
            return False
        except DuplicatedVariableError:
            sys.stderr.write('Duplicated variable %s at line %d:\n%s\n' % (
                strs[1], lineno + 1, code))
            return False
        except CurrentFunctionNoneError:
            sys.stderr.write(
                'Line %d does not belong to any function:\n%s\n' % (lineno + 1, code))
            return False

        return True

    def tableInsert(self, var, size=4, array=False):
        if var.isdigit():
            raise IRSyntaxError
        if var[0] == '&' or var[0] == '*':
            var = var[1:]
        elif var[0] == '#':
            test = int(var[1:])
            return
        if var in self.symTable:
            return
        self.functionDict[self.currentFunction].append(var)
        if self.currentFunction == 'main':
            self.symTable[var] = (
                self.offset, size, array)
            self.offset += size
        else:
            self.symTable[var] = (
                -1, size, array)

    def getValue(self, var):
        if var[0] == '#':
            return int(var[1:])
        else:
            if var[0] == '&':
                return self.symTable[var[1:]][0]
            if var[0] == '*':
                return self.mem[(self.mem[(self.symTable[var[1:]][0] / 4)] / 4)]
            return self.mem[(self.symTable[var][0] / 4)]

    def execute_code(self, code):
        self.instrCnt += 1
        try:
            if code[0] == 'READ':
                result = input()
                self.mem[self.symTable[code[1]][0] / 4] = result
            elif code[0] == 'WRITE':
                print self.getValue(code[1])
            elif code[0] == 'GOTO':
                self.ip = self.labelTable[code[1]]
            elif code[0] == 'IF':
                value1 = self.getValue(code[1])
                value2 = self.getValue(code[3])
                if eval(str(value1) + code[2] + str(value2)):
                    self.ip = self.labelTable[code[4]]
            elif code[0] == 'MOV':
                value = self.getValue(code[2])
                if code[1][0] == '*':
                    self.mem[self.mem[(
                        self.symTable[code[1][1:]][0] / 4)] / 4] = value
                else:
                    self.mem[self.symTable[code[1]][0] / 4] = value
            elif code[0] == 'ARITH':
                value1 = self.getValue(code[2])
                value2 = self.getValue(code[4])
                self.mem[self.symTable[code[1]][0] /
                         4] = eval(str(value1) + code[3] + str(value2))
            elif code[0] == 'RETURN':
                if len(self.callStack) == 0:
                    return 1
                returnValue = self.getValue(code[1])
                stackItem = self.callStack.pop()
                self.ip = stackItem[0]
                for key in stackItem[2].keys():
                    self.symTable[key] = stackItem[2][key]

                self.offset = stackItem[3]
                self.mem[self.symTable[stackItem[1]][0] / 4] = returnValue
                self.displayFunction.pop()
            elif code[0] == 'CALL':
                oldAddrs = dict()
                oldOffset = self.offset
                for key in self.functionDict[code[2]]:
                    oldAddrs[key] = self.symTable[key]
                    self.symTable[key] = (self.getNewAddr(
                        self.symTable[key][1]), self.symTable[key][1], self.symTable[key][2])

                self.callStack.append((self.ip, code[1], oldAddrs, oldOffset))
                self.ip = self.labelTable[code[2]]
                self.displayFunction.append(code[2])
            elif code[0] == 'ARG':
                self.argumentStack.append(self.getValue(code[1]))
            elif code[0] == 'PARAM':
                self.mem[self.symTable[code[1]][0] /
                         4] = self.argumentStack.pop()
        except IndexError:
            return 2

        return 0

    def getNewAddr(self, size):
        ret = self.offset
        self.offset = self.offset + size
        return ret

    def step(self):
        if self.ip == -1:
            self.stop()
            self.pauseRunning = True
            self.ip = self.entranceIP - 1

        self.ip += 1
        if self.ip < 0 or self.ip >= len(self.codes):
            sys.stderr.write(
                'Program Counter goes out of bound. The running program will be terminated instantly.\n')
            self.ip = -1
            self.pauseRunning = False
            return
        code = self.codes[self.ip]
        error_code = self.execute_code(code)
        if error_code == 1:
            sys.stderr.write(
                'Program has exited gracefully.\nTotal instructions = %d\n' % self.instrCnt)
            self.ip = -1
            self.pauseRunning = False
        elif error_code == 2:
            sys.stderr.write(
                'An error occurred at line %d: Illegal memory access\n' % (self.ip + 1))
            self.ip = -1
            self.pauseRunning = False


if __name__ == "__main__":
    input_file = sys.argv[1]
    e = IRSim()
    e.fileOpen(input_file)
    ret = e.run()
    if ret >= 0:
        ret = 0
    exit(ret)

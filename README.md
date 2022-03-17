# Introduction
Design a shell that support the following function.
1. Execution of commands.
2. Ordinary Pipe
3. Numbered Pipe
4. File Redirection
# Scenario
## Execution of commands
```shell
% printenv PATH
bin:.
```
## Ordinary Pipe
```
% removetag test.html | number
1
2 Test
3 This is a test program
4 for ras.
5
```

## Numbered Pipe
* |N means the STDOUT of the left hand side command will be piped to the first command
of the next N-th line.
* !N means both STDOUT and STDERR of the left hand side command will be piped to the
first command of the next N-th line.
```shell
% removetag test.html |1 # pipe STDOUT to the first command of next line
% number # STDIN is from the previous pipe (removetag)
1
2 Test
3 This is a test program
4 for ras.
5
% removetag test.html |2 # pipe STDOUT to the first command of next next line
2
% ls
bin npshell test1.txt test2.txt test.html
% number # STDIN is from the previous pipe (removetag)
1
2 Test
3 This is a test program
4 for ras.
5
```

## File Redirection
```shell
> % cat test.html > test1.txt
% cat test1.txt
Test
This is a <b>test</b> program
for ras.
```

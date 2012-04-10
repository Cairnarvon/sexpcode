#!/usr/bin/python

"""
SexpCode testing suite.
Write more tests and submit a pull request.
"""

import os
import subprocess

pass0 = [(0, r'test', 'test'),
         (0, r'{--- test ---}', 'test'),
         (0, r'before {--- test ---} after', 'before test after'),
         (0, r'{*0 test *0}', 'test'),
         (1, r' {--- abc', ''),
         (0, r'{v \{}', '{v {}'),
         (0, r'{{a b} c}', '{{a b} c}'),
         (0, r'{{{not verbatim}} a}', '{{{not verbatim}} a}'),
         (0, r'{\{ b \{}', 'b'),
         (1, r'{} }}', ''),
         (0, r'{--- \{ }}\}\\ {} --}---} ---}', r'\{ }}\}\\ {} --}---}')]
pass1 = [(0, '{define b a}', ''),
         (0, 'abc', 'abc'),
         (1, '{define a}', ''),
         (0, '{b i}', '{b i}'),
         (0, '{define x b}{x a}', '{b a}'),
         (0, '{define x b}{define y x}{y a}', '{b a}'),
         (0, '{define x b.i}{define y x.u}{i.y a}', '{i {b {i {u a}}}}'),
         (1, '{define x b}{undefine x}{x b}', ''),
         (0, '{undefine b}', ''),
         (0, '{undefine b}{b a}', '{b a}'),
         (1, '{define x y}{define y x}{x.y a}', ''),
         (0, '{define b i}{define i b}{b.i a}', '{i {i a}}'),
         (0, '{define x i}{define y x^2}{y a}', '{i {i a}}'),
         (0, '{define x i}{x italic}{define x b}{x bold}',
             '{i italic}{b bold}'),
         (0, '{define link url}{i.{link x}.b y}', '{i {url x {b y}}}'),
         (0, '{b.i.o test}', '{b {i {o test}}}'),
         (0, '{{url a} test}', '{url a test}'),
         (0, '{b.{url a}.i test}', '{b {url a {i test}}}'),
         (0, '{b.url.i a test}', '{b {url a {i test}}}'),
         (0, '{verbatim.i test}', '{i {verbatim test}}'),
         (0, '{verbatim.verbatim test}', '{verbatim test}'),
         (0, '{b^3.i*2.{code a}.verbatim.url b c}',
             '{b {b {b {i {i {code a {url b {verbatim c}}}}}}}}'),
         (1, '{b^4 a}', '')]
pass2 = [(0, 'Test', 'Test'),
         (0, '{b simple}', '<b>simple</b>'),
         (0, 'a {b b} c', 'a <b>b</b> c'),
         (0, '{b.i.u test}', '<b><i><u>test</u></i></b>'),
         (0, '{sup^3.sub*2.i^1.b*0 a}',
             '<sup><sup><sup><sub><sub><i>a</i></sub></sub></sup></sup></sup>'),
         (0, '{define b i}{define i b}{b.i x}', '<i><i>x</i></i>'),
         (1, '{define b x}{b x}', ''),
         (0, '{define b b}{undefine b}{b i}', '<b>i</b>'),
         (0, '{undefine b}', ''),
         (0, '{define i b}{i i}{undefine i}{i i}', '<b>i</b><i>i</i>'),
         (0, "{define link url}{b.url.i.{code '{a b c}}.url a b c d e f}",
             '<b><a href="a"><i><code title="a b c code"><a href="b">c d e f</a></code></i></a></b>'),
         (0, "{code '{Algorithmic Language Scheme} (fibs 10)}",
             '<code title="Algorithmic Language Scheme code">(fibs 10)</code>'),
         (0, '{b\ni}', '<b>i</b>')]

def test(tests, arg):
    for n, (r, i, o) in enumerate(tests):
        print "Test %d..." % (n + 1),
        p = subprocess.Popen(['./sexpcode', arg],
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        stdout, stderr = p.communicate(i)
        if stdout == o and p.returncode == r:
            print '\033[;32mOK\033[0m'
        else:
            print '\033[1;31mFAIL\033[0m'
            print '\t\033[1mInput:\033[0m'
            print '\t\t', i
            print '\t\033[1mExpected:\033[0m'
            print '\t\t', o
            if stdout:
                print '\t\033[1mGot:\033[0m'
                print '\t\t', stdout
            else:
                print '\t\033[1mBailed with message:\033[0m'
                print '\t\t', stderr,


if __name__ == '__main__':
    print 'Building executable...\033[2m'
    os.system('make')
    print '\033[0m'

    print '\033[1mPass 0\033[0m'
    test(pass0, '-pass0')

    print
    print '\033[1mPass 1\033[0m'
    test(pass1, '-pass1')

    print
    print '\033[1mPass 2\033[0m'
    test(pass2, '-pass2')

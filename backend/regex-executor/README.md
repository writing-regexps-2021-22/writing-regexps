# regex-executor

This is a library that can execute a regular expression on a given string,
returning both the matching results and the drilldown of the matching process
steps.

Currently, plans are to only support the naive backtracking algorithm. However,
alternative approaches are technically possible, for instance compiling
the regex to a DFA or an NFA and executing the latter.

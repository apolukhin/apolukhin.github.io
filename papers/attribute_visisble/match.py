#!/usr/bin/python

import re
import subprocess

def get_visible(compiler):
    output = subprocess.Popen(compiler + ' -g -fvisibility=hidden -shared main.cpp -o libmain.so && readelf -sW libmain.so |grep "GLOBAL" | less',
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE).stdout

    visible = set()
    while True:
        symbol = output.readline()
        if not symbol:
            break

        visible.update([vis for vis in parsed if vis in symbol])

    return visible

def extend_with_colors(items):
    ext = {}
    for key, value in items.iteritems():
        ext[key + "_color"] = 'green' if value else 'red'
        
    items.update(ext)


parsed = set()
with open("main.cpp") as f:
    for line in f:
        m = re.search("(cpp_[_a-z]+)", line)
        if m:
            parsed.add(m.group(1))

extern_linkage = set([
    'cpp_member_function',
    'cpp_function_inline',
    'cpp_static_member_function',
    'cpp_variable_noextern',
    'cpp_variable_extern',
    'cpp_member_function_inline',
    'cpp_function',
    'cpp_static_member_function_inline',
    'cpp_static_member_variable',
])

more_extern = set()
for i in extern_linkage:
    if "member" in i:
        more_extern.add('cpp_derived_nonvis_' + i[4:])
        more_extern.add('cpp_base_nonvis_' + i[4:])
        more_extern.add('cpp_derived_nonvis_private_' + i[4:])
        more_extern.add('cpp_base_nonvis_private_' + i[4:])
        more_extern.add('cpp_private_' + i[4:])

extern_linkage.update(more_extern)

gcc46_symbols = get_visible('g++-4.6')
gcc48_symbols = get_visible('g++-4.8')
gcc53_symbols = get_visible('g++-5')
clang34_symbols = get_visible('clang++')


result = []

pattern = (
    "<tr> "
        "<td>{name}</td> "
        "<td bgcolor='{gcc46_color}'>{gcc46}</td> "
        "<td bgcolor='{gcc48_color}'>{gcc48}</td> "
        "<td bgcolor='{gcc53_color}'>{gcc53}</td> "
        "<td bgcolor='{clang34_color}'>{clang34}</td> "
        "<td bgcolor='{computed_color}'>{computed}</td> "
        "<td>=</td> "
        "<td bgcolor='{linkage_color}'>{linkage}</td> "
        "<td bgcolor='{noinline_color}'>{noinline}</td> "
        "<td bgcolor='{visible_color}'>{visible}</td> "
    "</tr>"
)

for symbol in parsed:
    not_base_or_derived = not "derived" in symbol and not "base" in symbol
    is_extern = symbol in extern_linkage and not "anonymous_ns" in symbol
    not_inline = not "inline" in symbol
    computed_vis = not_base_or_derived and not_inline and is_extern
    
    s = {
        'name': symbol,
        'gcc46': symbol in gcc46_symbols,
        'gcc48': symbol in gcc48_symbols,
        'gcc53': symbol in gcc53_symbols,
        'clang34': symbol in clang34_symbols,
        'computed': computed_vis,
        'linkage': is_extern,
        'noinline': not_inline,
        'visible': not_base_or_derived,
    }
    extend_with_colors(s) 
    result.append( pattern.format(**s) )    

for l in sorted(result):
    print l

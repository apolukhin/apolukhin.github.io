#!/usr/bin/python

import re
import subprocess

# Compiles the main.cpp, extracts all the exported entities using readelf
def get_exported_entities(compiler, all_entities):
    popen_result = subprocess.Popen(compiler + ' -g -fvisibility=hidden -shared main.cpp -o libmain.so && readelf -sW libmain.so',
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    popen_result.wait()

    if popen_result.returncode != 0:
        print popen_result.stderr.read()
        exit(popen_result.returncode)

    output = popen_result.stdout
    exported = set()
    while True:
        line = output.readline()
        if not line:
            break

        if not " GLOBAL " in line:
            continue

        exported.update([e for e in all_entities if e in line])

    return exported


def extend_with_colors(items):
    ext = {}
    for key, value in items.iteritems():
        ext[key + "_color"] = 'green' if value else 'red'
        
    items.update(ext)


# Preprocesses the main.cpp, extracts all the entities with two underlines in the middle
def get_all_entities():
    popen_result = subprocess.Popen('g++ -E main.cpp',
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    popen_result.wait()

    if popen_result.returncode != 0:
        print popen_result.stderr.read()
        exit(popen_result.returncode)

    output = popen_result.stdout
    all_entities = []
    while True:
        line = output.readline()
        if not line:
            break

        all_entities += re.findall("([_a-z]+__[_a-z]+)", line)

    return list(set(all_entities))


def get_extern_linkage_entities():
    vis_suffix = [
        '__function',
        '__function_inline',
        '__variable_noextern',
        '__variable_extern',

        '__public_member_function',
        '__public_member_function_inline',
        '__public_static_member_function',
        '__public_static_member_function_inline',

        '__private_member_function',
        '__private_member_function_inline',
        '__private_static_member_function',
        '__private_static_member_function_inline',

        '__public_static_member_variable',
        '__private_static_member_variable',
    ]

    extern_linkage = set()
    for i in vis_suffix:
        if "member" in i:
            extern_linkage.add('global_ns__base_nonvis_class' + i)
            extern_linkage.add('global_ns__some_class' + i)
            extern_linkage.add('global_ns__derived_nonvis_class' + i)
            extern_linkage.add('global_ns__some_class__public_nested_class' + i)
            extern_linkage.add('global_ns__some_class__public_nested_class_not_inside' + i)
            extern_linkage.add('global_ns__some_class__private_nested_class' + i)
            extern_linkage.add('visible_ns__some_class' + i)
            extern_linkage.add('visible_ns__nested_ns__some_class' + i)
            extern_linkage.add('visible_ns__nested_ns__some_class__public_nested_class' + i)
            extern_linkage.add('visible_ns__nested_ns__some_class__public_nested_class_not_inside' + i)
        else:
            extern_linkage.add('global_ns' + i)
            extern_linkage.add('visible_ns' + i)
            extern_linkage.add('visible_ns__nested_ns' + i)

    return extern_linkage



def main():
    all_entities = get_all_entities()
    extern_linkage = get_extern_linkage_entities()

    gcc46_exports = get_exported_entities('g++-4.6', all_entities)
    gcc48_exports = get_exported_entities('g++-4.8', all_entities)
    gcc53_exports = get_exported_entities('g++-5', all_entities)
    clang34_exports = get_exported_entities('clang++', all_entities)

    result = []

    pattern = (
        "<tr> "
            "<td>{name}</td> "
            "<td bgcolor='{gcc46_color}'>{gcc46}</td> "
            "<td bgcolor='{gcc48_color}'>{gcc48}</td> "
            "<td bgcolor='{gcc53_color}'>{gcc53}</td> "
            "<td bgcolor='{clang34_color}'>{clang34}</td> "
            "<td style='border:none'> </td> "
            "<td bgcolor='{computed_color}'>{computed}</td> "
            "<td>=</td> "
            "<td bgcolor='{linkage_color}'>{linkage}</td> "
            "<td>&amp;&amp;</td> "
            "<td bgcolor='{noinline_color}'>{noinline}</td> "
            "<td>&amp;&amp;</td> "
            "<td bgcolor='{visible_color}'>{visible}</td> "
        "</tr>"
    )

    for entity in all_entities:
        not_base_or_derived = not "derived" in entity and not "base" in entity
        is_extern = entity in extern_linkage
        not_inline = not "inline" in entity
        computed_vis = not_base_or_derived and not_inline and is_extern
        
        s = {
            'name': entity,
            'gcc46': entity in gcc46_exports,
            'gcc48': entity in gcc48_exports,
            'gcc53': entity in gcc53_exports,
            'clang34': entity in clang34_exports,
            'computed': computed_vis,
            'linkage': is_extern,
            'noinline': not_inline,
            'visible': not_base_or_derived,
        }
        extend_with_colors(s) 
        result.append( pattern.format(**s) )

    for l in sorted(result):
        print l.replace('__','::').replace('global_ns', '').replace('anonymous_ns', '&lt;anonymous&gt;')


if __name__ == "__main__":
    main()

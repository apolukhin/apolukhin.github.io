# Script to generate auto-updatable status table for all the libraries I maintain.

find . -name "README.md" -print0 | xargs -0 grep '\[!\[' | grep 'apolukhin' | sed -n 's:./\(.*\)/README.md\:\(.*\):<tr><td>\u\1 \2</td></tr>:gp' | sed 's:|:</td><td>:g' | sed 's:!\[\([a-zA-Z ]*\)\](\([^)]*\)):<img src="\2" alt="\1">:g' | sed 's:\[\([^]]*\)\](\([^)]*\)):<a href="\2">\1</a>:g' | sort


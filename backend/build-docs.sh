#!/bin/bash
set -eo pipefail
shopt -s failglob

function print_usage_and_exit {
    echo "Usage: $0 <project-name> <output-dir>" >&2
    exit 1
}

project="$1"
output_dir="$2"
if [[ -z $project || -z $output_dir ]]; then
    print_usage_and_exit
fi

output_dir="$(realpath -- "$output_dir")"
cd "$project"
doxygen doxygen.conf
rm -rf -- "$output_dir"
mkdir -p -- "$output_dir/"
echo 'Copying API reference documentation'
cp -r --reflink=auto -- "build/docs/html/"* "$output_dir/"
echo 'Compiling readme'
pandoc README.md -o "$output_dir/readme.tmp.html" --highlight-style pygments -s --metadata "title=$project"
cat -- "$output_dir/readme.tmp.html" \
    | sed -e 's%</head>%<link rel="stylesheet" href="../style.css"></head>%' \
    | sed -e "s%<body>%<body><a href=\"../index.html\">[Index]</a> · <a href=\"index.html\">[API reference (HTML)]</a> · <a href=\"$project-api-reference.pdf\">[API reference (PDF)]</a><br>%" > "$output_dir/readme.html"
rm -f -- "$output_dir/readme.tmp.html"
echo 'Compiling PDF documentation'
pushd build/docs/latex/
make
cp --reflink=auto -- refman.pdf "$output_dir/$project-api-reference.pdf"
popd
echo 'Done'

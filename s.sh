#!/bin/bash
# z
set -e

select_project() {
    if [[ ! -f CMakeLists.txt ]]; then
        exit 1
    fi
    projects=$(grep -oP '(?<=add_subdirectory\()[\w]+' CMakeLists.txt)
    if [[ -z "$projects" ]]; then
        echo "No subdirectories found in CMakeLists.txt."
        exit 1
    fi
    printf "🔥 Projects: \n" >&2
    while read -r project; do
        printf "\t %s\n" "$project" >&2
    done <<< "$projects"
    mapfile -t project_array <<< "$projects"
    printf "\n" >&2
    PS3="Select: "
    select project in "${project_array[@]}"; do
        if [[ -n "$project" ]]; then
            printf "Selected: %s\n" "$project" >&2
            break
        else
            printf "Invalid selection. Please try again.\n" >&2
        fi
    done
    echo "$project"
}

case "$1" in
    r)
        project=$(select_project)
        if [[ ! -d build ]]; then
            cmake -S . -B build
        fi
        cmake --build build --target "$project"
        echo "🔥 Built $project. Now executing."
        "./build/$project/$project"
        ;;
    b)
        project=$(select_project)
        if [[ ! -d build ]]; then
            cmake -S . -B build
        fi
        cmake --build build --target "$project"
        echo "🔥 Built $project."
        ;;
    t)
        project=$(select_project)
        if [[ ! -d build ]]; then
            cmake -S . -B build
        fi
        test_target="${project}_tests"
        cmake --build build --target "$test_target"
        echo "🔥 Built $test_target. Running tests."
        ctest --test-dir "build/$project" --output-on-failure
        ;;
    f)
        find . -type f \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) \
            ! -path "./build/*" \
            | xargs -r clang-format -i
        echo "🔥 Formatted all C/C++ files."
        ;;
    *)
        echo "Usage: z <command>"
        echo "  r   build and run a project"
        echo "  b   build only"
        echo "  t   build and run tests"
        echo "  f   format all C/C++ files"
        ;;
esac
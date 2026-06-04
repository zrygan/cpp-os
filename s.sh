#!/bin/bash
# z

set -e 

case "$1" in
    r)
        if [[ ! -f CMakeLists.txt ]]; then
            exit 1
        fi

        projects=$(grep -oP '(?<=add_subdirectory\()[\w]+' CMakeLists.txt)

        if [[ -z "$projects" ]]; then
            echo "No subdirectories found in CMakeLists.txt."
            exit 1
        fi

        printf "🔥 Projects: \n"
        while read -r project; do
            printf "\t %s\n" "$project"
        done <<< "$projects"

        mapfile -t project_array <<< "$projects"

        printf "\n"
        PS3="Select: "

        select project in "${project_array[@]}"; do
            if [[ -n "$project" ]]; then
                printf "Selected: %s\n" "$project"
                break
            else
                printf "Invalid selection. Please try again.\n"
            fi
        done

        if [[ ! -d build ]]; then
            cmake -S . -B build
        fi

        cmake --build build --target "$project"
        echo "🔥 Built $project. Now executing."
        "./build/$project/$project"
        ;;
    f)
        # FIX: Added -r to xargs to prevent errors if find output is empty
        find . -type f \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) \
            ! -path "./build/*" \
            | xargs -r clang-format -i
        echo "🔥 Formatted all C/C++ files."
        ;;
    *)
        echo "Usage: z <command>"
        echo "  r   build and run a project"
        echo "  f   format all C/C++ files"
        ;;
esac

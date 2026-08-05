find . \( -name "*.cpp" -o -name "*.h" \) -exec wc -l {} +

find . \( -name "*.h" -o -name "*.cpp" \) -exec clang-format -i {} +
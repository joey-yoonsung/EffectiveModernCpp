
add_executable(Item1 Item1.cpp)

enable_testing()

add_test(Item1 Item1 -o "Item1.xml" -r "xml")

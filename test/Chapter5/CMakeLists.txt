
add_executable(Item23 Item23.cpp)

enable_testing()

add_test(Item23 Item23 -o "Item23.xml" -r "xml")

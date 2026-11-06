# CMakeLists.txt для модуля длинной арифметики
file(GLOB_RECURSE BIGNUM_SOURCES "*.cpp")

add_library(bignum_lib STATIC ${BIGNUM_SOURCES})

# Настраиваем пути поиска заголовков, включая все подпапки Library
target_include_directories(bignum_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Library/Service_Names
    ${CMAKE_CURRENT_SOURCE_DIR}/Library/Int_Number
    ${CMAKE_CURRENT_SOURCE_DIR}/Library/Error
    ${CMAKE_CURRENT_SOURCE_DIR}/Library/Real_Number
    ${CMAKE_CURRENT_SOURCE_DIR}/Library/Thread_Pool
    ${CMAKE_CURRENT_SOURCE_DIR}/Library/Quotient_Number
)

# Применяем ваши флаги оптимизации
target_compile_options(bignum_lib PRIVATE 
    -O3 
    -march=native 
    -ftree-vectorize
)

# Minimum required version fo CMake. Can be modified at your own risk.
# @see https://cmake.org/download/
cmake_minimum_required(VERSION 3.28.3)

# Name of the project and language to use (C, in this case).
project(Flex-Bison-Compiler C)

# Selects the best strategy according to the available compiler in the system.
# @see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
	message(NOTICE "The C compiler is GCC.")

	# Compiles the parser with Bison, generating counter-examples in case S/R or R/R conflicts exists.
	add_custom_command(
		OUTPUT ../src/main/c/frontend/syntactic-analysis/BisonParser.c ../src/main/c/frontend/syntactic-analysis/BisonParser.h
		COMMAND ../src/main/bash/bison.sh
	)

	# Options for GCC.
	add_compile_options(-fsanitize=address)
	add_compile_options(-O3)
	add_compile_options(-static-libgcc)
	add_compile_options(-std=gnu99)
	# Queremos que el proyecto compile sin warnings, asi que los activamos para
	# todo nuestro codigo (la unica excepcion es el codigo generado, mas abajo).
	add_compile_options(-Wall -Wextra)
	add_link_options(-fsanitize=address)

	# Compiles the scanner with Flex.
	add_custom_command(
		OUTPUT ../src/main/c/frontend/lexical-analysis/FlexScanner.c ../src/main/c/frontend/lexical-analysis/FlexScanner.h
		COMMAND ../src/main/bash/flex.sh
		DEPENDS ../src/main/c/frontend/syntactic-analysis/BisonParser.c ../src/main/c/frontend/syntactic-analysis/BisonParser.h
	)

	# Defines the entry-point of the application, and the source-codes (*.c extension).
	# The header files (*.h extension), are automatically included from the source-codes.
	add_executable(Flex-Bison-Compiler
		src/main/c/backend/code-generation/Generator.c
		src/main/c/backend/code-generation/reports/HtmlReport.c
		src/main/c/backend/code-generation/reports/PdfReport.c
		src/main/c/backend/code-generation/reports/ReportModel.c
		src/main/c/backend/code-generation/reports/TextReport.c
		src/main/c/backend/semantic-analysis/SemanticAnalyzer.c
		src/main/c/EntryPoint.c
		src/main/c/frontend/Frontend.c
		src/main/c/frontend/lexical-analysis/FlexActions.c
		src/main/c/frontend/lexical-analysis/FlexScanner.c
		src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.c
		src/main/c/frontend/syntactic-analysis/BisonActions.c
		src/main/c/frontend/syntactic-analysis/BisonParser.c
		src/main/c/support/configuration/Environment.c
		src/main/c/support/io/EmitSql.c
		src/main/c/support/language/DateUtils.c
		src/main/c/support/language/String.c
		src/main/c/support/logging/Logger.c
		# Add more *.c files if needed (otherwise, they won't be compiled).
		# ...
	)

	# El scanner (Flex) y el parser (Bison) son codigo GENERADO, no escrito por
	# nosotros: se regeneran en cada build y arrastran warnings propios de Flex/
	# Bison que no nos toca arreglar (entre ellos el stub yyerror del template
	# base, que Bison copia dentro de BisonParser.c). Los silenciamos con -w para
	# que -Wall/-Wextra sigan siendo utiles sobre nuestro codigo real.
	set_source_files_properties(
		src/main/c/frontend/lexical-analysis/FlexScanner.c
		src/main/c/frontend/syntactic-analysis/BisonParser.c
		PROPERTIES COMPILE_OPTIONS "-w"
	)

	# Link final project and libraries.
	target_link_libraries(Flex-Bison-Compiler)
else ()
	message(NOTICE "The C compiler is unknown.")
endif ()

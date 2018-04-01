################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Bytes.obj: ../Bytes.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/home/tim/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.4.LTS/bin/cl430" -vmsp --data_model=restricted -O4 --use_hw_mpy=F5 --include_path="/home/tim/ti/ccsv7/ccs_base/msp430/include" --include_path="/home/tim/Dropbox/Thesis/Dalvik16_Wrapper/Interpreter" --include_path="/home/tim/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.4.LTS/include" --advice:power="all" --advice:hw_config="all" --define=__MSP430FR6989__ -g --c99 --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="Bytes.d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

interpreter.obj: ../interpreter.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/home/tim/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.4.LTS/bin/cl430" -vmsp --data_model=restricted -O4 --use_hw_mpy=F5 --include_path="/home/tim/ti/ccsv7/ccs_base/msp430/include" --include_path="/home/tim/Dropbox/Thesis/Dalvik16_Wrapper/Interpreter" --include_path="/home/tim/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.4.LTS/include" --advice:power="all" --advice:hw_config="all" --define=__MSP430FR6989__ -g --c99 --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="interpreter.d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/home/tim/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.4.LTS/bin/cl430" -vmsp --data_model=restricted -O4 --use_hw_mpy=F5 --include_path="/home/tim/ti/ccsv7/ccs_base/msp430/include" --include_path="/home/tim/Dropbox/Thesis/Dalvik16_Wrapper/Interpreter" --include_path="/home/tim/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.4.LTS/include" --advice:power="all" --advice:hw_config="all" --define=__MSP430FR6989__ -g --c99 --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="main.d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '



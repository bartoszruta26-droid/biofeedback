# Regression tests for the existing non-GUI components.
# The executable targets are declared by the root CMakeLists.txt.
if(TARGET auth_regression)
    add_test(NAME auth_regression COMMAND auth_regression)
endif()

if(TARGET data_manager_regression)
    add_test(NAME data_manager_regression COMMAND data_manager_regression)
endif()

if(TARGET rep_detector_regression)
    add_test(NAME rep_detector_regression COMMAND rep_detector_regression)
endif()

if(TARGET serial_subscription_regression)
    add_test(NAME serial_subscription_regression COMMAND serial_subscription_regression)
endif()

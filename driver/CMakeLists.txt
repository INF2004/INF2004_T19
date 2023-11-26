# CMakeLists.txt in the driver directory

# Set additional directory-specific settings here

# Add the subdirectories for specific drivers/components
add_subdirectory(encoder)
add_subdirectory(irline)
add_subdirectory(magnetometer)
add_subdirectory(motor)
add_subdirectory(ui)
add_subdirectory(ultrasonic)
# add_subdirectory(wifi)

# Add any additional targets or source files specific to this directory
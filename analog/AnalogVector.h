/**
 * @file AnalogVector.h
 * @brief This file contains the declaration and implementation of the AnalogVector class with quantization functionality.
 */

#ifndef ANALOG_VECTOR_H
#define ANALOG_VECTOR_H

#include <cmath>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <limits>
#include <type_traits>
#include <exception>  // For std::bad_alloc

/**
 * @class AnalogVector
 * @brief Represents a vector compatible with MVM analog intrinsic calls.
 * @tparam T Data type of the elements in the vector (float, int8_t, int16_t, int32_t).
 */
template <typename T, typename qT = T>
class AnalogVector {
public:
    /**
     * @brief Constructor of the AnalogVector class without an array.
     * @param length Length of the host array.
     */
    AnalogVector(uint32_t length)
        : host_arr(nullptr),
          host_length(length),
          device_arr(nullptr),
          device_length(DEVICE_COLS),
          owns_host_arr(true),
          scale_factor(1.0f) {
        static_assert(std::is_arithmetic<T>::value, "AnalogVector requires arithmetic data type");

        // Allocate memory for host_arr
        try {
            host_arr = new T[host_length]();
        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed for host_arr: " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }

        // Allocate memory for device_arr
        try {
            device_arr = new T[device_length]();
        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed for device_arr: " << e.what() << std::endl;
            delete[] host_arr;  // Free previously allocated memory before exiting
            exit(EXIT_FAILURE);
        }
    }

    /**
     * @brief Constructor of the AnalogVector class using an array.
     * @param arr Pointer to the host array.
     * @param length Length of the host array.
     */
    AnalogVector(T* arr, uint32_t length)
        : host_arr(arr),
          host_length(length),
          device_arr(nullptr),
          device_length(DEVICE_COLS),
          owns_host_arr(false),
          scale_factor(1.0f) {
        static_assert(std::is_arithmetic<T>::value, "AnalogVector requires arithmetic data type");

        // Allocate memory for device_arr
        try {
            device_arr = new qT[device_length]();
        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed for device_arr: " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    /**
     * @brief Destructor to clean up allocated device array memory.
     */
    ~AnalogVector() {
        if (owns_host_arr && host_arr != nullptr) {
            delete[] host_arr;
        }
        if (device_arr != nullptr) {
            delete[] device_arr;
        }
    }

    /**
     * @brief Transfers data from the host array to the device array.
     *
     * For floating-point and integral types, performs a direct copy.
     */
    void direct_transfer_to_device() {
        if (device_arr == nullptr || host_arr == nullptr) {
            std::cerr << "Error: device_arr or host_arr is null. Cannot transfer data." << std::endl;
            return;
        }

        // Perform direct copy
        for (uint32_t i = 0; i < host_length; i++) {
            device_arr[i] = host_arr[i];
        }
    }

    /**
     * @brief Transfers data from the host array to the device array with quantization.
     *
     * This method is only for floating-point types. It quantizes the data to the specified integral type.
     * @tparam qT The integral type to quantize to (e.g., int8_t, int16_t).
     */
    void quantize_transfer_to_device() {
        if (!(std::is_floating_point<T>::value)) {
            std::cerr << "Error: Quantization is only applicable to floating-point types." << std::endl;
        }

        if (device_arr) {
            delete[] device_arr;
            device_arr = nullptr;
        }

        try {
            // Allocate memory for device_arr using new[]
            device_arr = new qT[device_length]();
        } catch (const std::bad_alloc&) {
            std::cerr << "Memory allocation failed for device_arr" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (host_arr == nullptr) {
            std::cerr << "Error: host_arr is null. Cannot transfer data." << std::endl;
            return;
        }

        // Identify the scaling factor
        double max_abs_value = 0.0f;
        for (uint32_t i = 0; i < host_length; i++) {
            double tmp_val = std::abs(host_arr[i]);
            if (tmp_val > max_abs_value) {
                max_abs_value = tmp_val;
            }
        }
        scale_factor = (max_abs_value == 0.0f) ? 1.0f : max_abs_value;

        // Determine quantization limits
        qT max_type_limit = std::numeric_limits<qT>::max();
        qT min_type_limit = std::numeric_limits<qT>::min();

        for (uint32_t i = 0; i < host_length; i++) {
            double scaled_value = static_cast<double>(host_arr[i] / scale_factor * max_type_limit);

            // Clamp the scaled value to the range of quant_type
            if (scaled_value > static_cast<double>(max_type_limit)) {
                scaled_value = static_cast<double>(max_type_limit);
            }
            else if (scaled_value < static_cast<double>(min_type_limit)) {
                scaled_value = static_cast<double>(min_type_limit);
            }

            device_arr[i] = static_cast<qT>(std::llround(scaled_value));
        }
    }

    void transfer_to_device() {
        if (std::is_same<T, qT>::value) {
            direct_transfer_to_device();
        } else {
            quantize_transfer_to_device();
        }
    }

    void direct_transfer_to_host() {
        for (uint32_t i = 0; i < host_length; i++) {
            host_arr[i] = device_arr[i];
        }
    }

    void dequantize_transfer_to_host(double sf) {
        for (uint32_t i = 0; i < host_length; i++) {
            host_arr[i] = static_cast<T>(device_arr[i]) * sf;
        }
    }

    void transfer_to_host(double sf = 1.0) {
        if (std::is_same<T, qT>::value) {
            direct_transfer_to_host();
        } else {
            dequantize_transfer_to_host(sf);
        }
    }

    /**
     * @brief Returns the device array.
     * @return Pointer to the device array.
     */
    qT* get_device_arr() const {
        return device_arr;
    }

    /**
     * @brief Gets the scale factor used in quantization.
     * @return The scale factor.
     */
    float get_scale_factor() const {
        if (std::is_same<T, qT>::value) {
            return scale_factor;
        } else {
            return scale_factor / std::numeric_limits<qT>::max();
        }
    }

    /**
     * @brief Prints the properties and content of the host and device arrays.
     */
    void print() const {
        std::cout << "######## AnalogLibrary Print ########" << std::endl;
        if (device_arr == nullptr) {
            std::cout << "Vector not initialized." << std::endl;
            return;
        }

        std::cout << "Vector:" << std::endl;

        // Print host array
        std::cout << "\tHost Array Length: " << host_length << std::endl;
        std::cout << "\tHost Array:" << std::endl;
        std::cout << "\t\t";
        for (uint32_t i = 0; i < host_length; i++) {
            if (std::is_integral<T>::value) {
                std::cout << std::setw(6) << static_cast<int64_t>(host_arr[i]) << " ";
            } else {
                std::cout << std::setw(6) << static_cast<float>(host_arr[i]) << " ";
            }
        }
        std::cout << std::endl;

        // Print device array
        std::cout << "\tDevice Array Length: " << device_length << std::endl;
        std::cout << "\tDevice Array:" << std::endl;
        std::cout << "\t\t";
        for (uint32_t i = 0; i < device_length; i++) {
            if (std::is_integral<T>::value) {
                std::cout << std::setw(6)
                          << static_cast<int64_t>(device_arr[i]) << " ";
            } else {
                std::cout << std::setw(6)
                          << static_cast<float>(device_arr[i]) << " ";
            }
        }

        std::cout << std::endl;
        std::cout << "#####################################" << std::endl;
        std::cout << std::endl;
    }

private:
    T* host_arr;            ///< Pointer to the host array.
    uint32_t host_length;   ///< Length of the host array.
    qT* device_arr;         ///< Pointer to the device array (can be of different types).
    uint32_t device_length; ///< Length of the device array.
                            
    double scale_factor;    ///< The scale factor for quantization.
                             
    bool owns_host_arr;     ///< Whether this object owns and should delete the host array.
};

#endif // ANALOG_VECTOR_H

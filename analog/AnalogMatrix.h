/**
 * @file AnalogMatrix.h
 * @brief This file contains the declaration and implementation of the AnalogMatrix class.
 */

#ifndef ANALOG_MATRIX_H
#define ANALOG_MATRIX_H

#include <cmath>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <limits>
#include <type_traits>
#include <new>        // For std::nothrow
#include <exception>  // For std::bad_alloc

/**
 * @class AnalogMatrix
 * @brief Represents a matrix compatible with MVM analog intrinsic calls.
 * @tparam T Data type of the elements in the matrix (float, int8_t, int16_t, int32_t).
 */
template <typename T, typename qT = T>
class AnalogMatrix {
public:
    /**
     * @brief Constructor for the AnalogMatrix class.
     * @param mat 2D array representing the host matrix.
     * @param rows Number of rows in the host matrix.
     * @param cols Number of columns in the host matrix.
     */
    AnalogMatrix(T** mat, uint16_t rows, uint16_t cols)
        : host_mat(mat),
          host_rows(rows),
          host_cols(cols),
          device_mat(nullptr),
          device_rows(DEVICE_ROWS),
          device_cols(DEVICE_COLS),
          scale_factor(1.0f),
          owns_host_mat(false) 
    {
        static_assert(std::is_arithmetic<T>::value, "AnalogMatrix requires arithmetic data type");
        try {
            // Allocate memory for device_mat using new[]
            device_mat = new qT[device_rows * device_cols]();
        } catch (const std::bad_alloc&) {
            std::cerr << "Memory allocation failed for device_mat" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    /**
     * @brief Constructor accepting a 1D array*.
     * @param mat 1D array representing the host matrix.
     * @param rows Number of rows in the host matrix.
     * @param cols Number of columns in the host matrix.
     */
    AnalogMatrix(T* mat, uint16_t rows, uint16_t cols)
        : host_mat(nullptr),
          host_rows(rows),
          host_cols(cols),
          device_mat(nullptr),
          device_rows(DEVICE_ROWS),
          device_cols(DEVICE_COLS),
          scale_factor(1.0f),
          owns_host_mat(true)
    {
        static_assert(std::is_arithmetic<T>::value, "AnalogMatrix requires arithmetic data type");
        try {
            // Allocate memory for host_mat as a contiguous 2D matrix
            host_mat = new T*[rows];
            for (uint16_t i = 0; i < rows; ++i) {
                host_mat[i] = new T[cols];  // allocate each row
            }
        } catch (const std::bad_alloc&) {
            std::cerr << "Memory allocation failed for host_mat" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Copy data from the 1D array (mat) into the 2D matrix (host_mat)
        for (uint16_t i = 0; i < rows; ++i) {
            for (uint16_t j = 0; j < cols; ++j) {
                host_mat[i][j] = mat[i * cols + j];
            }
        }

        try {
            // Allocate memory for device_mat using new[]
            device_mat = new qT[device_rows * device_cols]();
        } catch (const std::bad_alloc&) {
            std::cerr << "Memory allocation failed for device_mat" << std::endl;
            // Free previously allocated memory before exiting
            delete[] host_mat;
            exit(EXIT_FAILURE);
        }
    }

    /**
     * @brief Destructor to clean up allocated device matrix memory.
     */
    ~AnalogMatrix() {
        // Deallocate host_mat only if it was allocated internally
        if (owns_host_mat && host_mat) {
            delete[] host_mat;
        }

        // Deallocate device_mat
        if (device_mat) {
            delete[] device_mat;
        }
    }

    /**
     * @brief Transfers data from the host array to the device array.
     *
     * For floating-point types, performs direct copy.
     * For integral types, performs direct copy.
     */
    void direct_transfer_to_device() {
        if (device_mat == nullptr || host_mat == nullptr) {
            std::cerr << "Error: device_mat or host_mat is null. Cannot transfer data." << std::endl;
            return;
        }
        // For integral types, perform direct copy
        for (uint16_t i = 0; i < host_rows; i++) {
            for (uint16_t j = 0; j < host_cols; j++) {
                uint16_t device_index = i * device_cols + j;
                device_mat[device_index] = host_mat[i][j];
            }
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

        if (device_mat) {
            delete[] device_mat;
            device_mat = nullptr;
        }

        try {
            // Allocate memory for device_mat using new[]
            device_mat = new qT[device_rows * device_cols]();
        } catch (const std::bad_alloc&) {
            std::cerr << "Memory allocation failed for device_mat" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (host_mat == nullptr) {
            std::cerr << "Error: host_mat is null. Cannot transfer data." << std::endl;
            return;
        }

        // Identify the scaling factor
        double max_abs_value = 0.0f;
        for (uint16_t i = 0; i < host_rows; i++) {
            for (uint16_t j = 0; j < host_cols; j++) {
                double tmp_val = std::abs(host_mat[i][j]);
                if (tmp_val > max_abs_value) {
                    max_abs_value = tmp_val;
                }
            }
        }
        scale_factor = (max_abs_value == 0.0f) ? 1.0f : max_abs_value;

        // Determine quantization limits
        qT max_type_limit = std::numeric_limits<qT>::max();
        qT min_type_limit = std::numeric_limits<qT>::min();

        for (uint16_t i = 0; i < host_rows; i++) {
            for (uint16_t j = 0; j < host_cols; j++) {
                uint16_t device_index = i * device_cols + j;
                double scaled_value = static_cast<double>(host_mat[i][j] / scale_factor * max_type_limit);

                // Clamp the scaled value to the range of quant_type
                if (scaled_value > static_cast<double>(max_type_limit)) {
                    scaled_value = static_cast<double>(max_type_limit);
                }
                else if (scaled_value < static_cast<double>(min_type_limit)) {
                    scaled_value = static_cast<double>(min_type_limit);
                }

                device_mat[device_index] = static_cast<qT>(std::llround(scaled_value));
            }
        }
    }

    void transfer_to_device() {
        if (std::is_same<T, qT>::value) {
            direct_transfer_to_device();
        } else {
            quantize_transfer_to_device();
        }
    }

    /**
     * @brief Returns the device matrix.
     * @return Pointer to the device matrix.
     */
    qT* get_device_mat() const {
        return device_mat;
    }

    /**
     * @brief Gets the scale factor used in quantization.
     * @return The scale factor.
     */
    double get_scale_factor() const {
        if (std::is_same<T, qT>::value) {
            return scale_factor;
        } else {
            return scale_factor / std::numeric_limits<qT>::max();
        }
    }

    /**
     * @brief Prints the properties and content of the device matrix.
     */
    void print() const {
        std::cout << "######## AnalogLibrary Print ########" << std::endl;
        if (device_mat == nullptr) {
            std::cout << "Matrix not transferred to device." << std::endl;
            return;
        }

        std::cout << "Matrix:" << std::endl;
        std::cout << "\tHost Size: " << host_rows << "x" << host_cols << std::endl;
        std::cout << "\tHost Matrix: " << std::endl;
        for (uint16_t i = 0; i < host_rows; i++) {
            std::cout << "\t\t";
            for (uint16_t j = 0; j < host_cols; j++) {
                if (std::is_integral<T>::value) {
                    std::cout << std::setw(6) << static_cast<int64_t>(host_mat[i][j]) << " ";
                } else {
                    std::cout << std::setw(6) << host_mat[i][j] << " ";
                }
            }
            std::cout << std::endl;
        }

        std::cout << "\tDevice Size: " << device_rows << "x" << device_cols << std::endl;
        std::cout << "\tDevice Matrix: " << std::endl;
        for (uint16_t i = 0; i < device_rows; i++) {
            std::cout << "\t\t";
            for (uint16_t j = 0; j < device_cols; j++) {
                uint16_t index = i * device_cols + j;
                if (std::is_integral<T>::value) {
                    std::cout << std::setw(6)
                              << static_cast<int64_t>(device_mat[index]) << " ";
                } else {
                    std::cout << std::setw(6)
                              << static_cast<float>(device_mat[index]) << " ";
                }
            }
            std::cout << std::endl;
        }

        std::cout << "#####################################" << std::endl;
        std::cout << std::endl;
    }

private:
    T** host_mat;         ///< Pointer to the host matrix.
    uint16_t host_rows;   ///< Number of rows in the host matrix.
    uint16_t host_cols;   ///< Number of columns in the host matrix.
    qT* device_mat;        ///< Pointer to the device matrix.
    uint16_t device_rows; ///< Number of rows in the device matrix.
    uint16_t device_cols; ///< Number of columns in the device matrix.

    double scale_factor;   ///< The scale factor for quantization.

    bool owns_host_mat;   ///< Indicates if this object owns the host_mat memory
};

#endif // ANALOG_MATRIX_H

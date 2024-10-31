/**
 * @file AnalogContext.h
 * @brief This file contains the declaration and implementation of the AnalogContext class.
 */

#ifndef ANALOG_CONTEXT_H
#define ANALOG_CONTEXT_H

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <new>

/**
 * @class AnalogContext
 * @brief The AnalogContext class keeps track of array scale factors.
 */
class AnalogContext {
public:
    /**
     * @brief Constructor of the AnalogContext class.
     * @param num_arrays The number of arrays for which scales are maintained.
     */
    AnalogContext(uint32_t num_arrays)
        : num_arrays(num_arrays),
          matrix_scales(nullptr),
          vector_scales(nullptr),
          output_scales(nullptr) {
        // Allocate memory for the scales using new
        matrix_scales = new double[num_arrays];
        vector_scales = new double[num_arrays];
        output_scales = new double[num_arrays];

        if (!matrix_scales || !vector_scales || !output_scales) {
            std::cerr << "Memory allocation failed in AnalogContext constructor" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Initialize each element to 1.0
        for (uint32_t i = 0; i < num_arrays; ++i) {
            matrix_scales[i] = 1.0f;
            vector_scales[i] = 1.0f;
            output_scales[i] = 1.0f;
        }
    }

    /**
     * @brief Destructor of the AnalogContext class.
     */
    ~AnalogContext() {
        delete[] matrix_scales;
        delete[] vector_scales;
        delete[] output_scales;
    }

    // Methods to set and get scales
    void set_matrix_scale(double value, uint32_t index) {
        if (index < num_arrays) {
            matrix_scales[index] = value;
        }
    }

    void set_vector_scale(double value, uint32_t index) {
        if (index < num_arrays) {
            vector_scales[index] = value;
        }
    }

    void compute_scale(uint32_t index) {
        if (index < num_arrays) {
            output_scales[index] = matrix_scales[index] * vector_scales[index];
        }
    }

    double get_scale(uint32_t index) const {
        if (index < num_arrays) {
            return output_scales[index];
        }
        return 0.0f;
    }

    void print() const {
        std::cout << "Matrix Scale Factors:" << std::endl;
        for (uint32_t i = 0; i < num_arrays; ++i) {
            std::cout << "Index " << i << ": " << matrix_scales[i] << std::endl;
        }

        std::cout << "Vector Scale Factors:" << std::endl;
        for (uint32_t i = 0; i < num_arrays; ++i) {
            std::cout << "Index " << i << ": " << vector_scales[i] << std::endl;
        }

        std::cout << "Output Scale Factors:" << std::endl;
        for (uint32_t i = 0; i < num_arrays; ++i) {
            std::cout << "Index " << i << ": " << output_scales[i] << std::endl;
        }
    }

private:
    uint32_t num_arrays;    ///< Number of arrays
    double* matrix_scales;   ///< Array of matrix scale factors
    double* vector_scales;   ///< Array of vector scale factors
    double* output_scales;   ///< Array of output scale factors
};

#endif // ANALOG_CONTEXT_H

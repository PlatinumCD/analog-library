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

#include "analogType.h"

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
          matrices(nullptr),
          input_vectors(nullptr),
          output_vectors(nullptr) {
        // Allocate memory for the scales using new
        matrices = new AnalogType*[num_arrays];
        input_vectors = new AnalogType*[num_arrays];
        output_vectors = new AnalogType*[num_arrays];

        for (int i = 0; i < num_arrays; i++) {
            matrices[i] = nullptr;
            input_vectors[i] = nullptr;
            output_vectors[i] = nullptr;
        }

        if (!matrices || !input_vectors || !output_vectors) {
            std::cerr << "Memory allocation failed in AnalogContext constructor" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void set_matrix(AnalogType* mat, uint32_t tile_id) {
        matrices[tile_id] = mat;
    }

    AnalogType* get_matrix(uint32_t tile_id) {
        return matrices[tile_id];
    }

    void set_input_vector(AnalogType* vec, uint32_t tile_id) {
        input_vectors[tile_id] = vec;
    }

    AnalogType* get_input_vector(uint32_t tile_id) {
        return input_vectors[tile_id];
    }

    void compute_update(uint32_t tile_id) {
        output_vectors[tile_id] = input_vectors[tile_id];
    }

    void move_vector(uint32_t tile_id, uint32_t tile_id_new) {
        input_vectors[tile_id_new] = output_vectors[tile_id];
    }

    /**
     * @brief Destructor of the AnalogContext class.
     */
    ~AnalogContext() {
        delete[] matrices;
        delete[] input_vectors;
        delete[] output_vectors;
    }


private:
    uint32_t num_arrays;    ///< Number of arrays
    AnalogType** matrices;
    AnalogType** input_vectors;
    AnalogType** output_vectors;
};

#endif // ANALOG_CONTEXT_H

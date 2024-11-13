/**
 * @file analogOperations.h
 * @brief This file defines several operations related to Analog data types.
 */

#ifndef ANALOG_OPERATIONS_H
#define ANALOG_OPERATIONS_H

#include <cstdint>
#include "analogMatrix.h"
#include "analogVector.h"
#include "analogContext.h"

/**
 * @brief Sets a matrix to a specified tile.
 * @param ctx The analog context managing the scales.
 * @param mat The matrix to set to the tile.
 * @param tile_id The ID of the tile to set the matrix.
 * @return The status flag indicating whether the operation was successful or not.
 */
template <typename T, typename qT = T>
uint16_t mvm_set_matrix(AnalogContext &ctx, AnalogMatrix<T, qT> &mat, uint16_t tile_id) {
    mat.transfer_to_device(); // Transfer the matrix to device (quantize if integral)
                              //
    ctx.set_matrix(&mat, tile_id); // Set the matrix scale in the context

    qT* data = mat.get_device_mat(); // Get the pointer to the device matrix data
    uint16_t status_flag = 0;

    asm volatile (
        "mvm.set %0, %1, %2"
        : "=r" (status_flag)
        : "r"(data), "r"(tile_id)
        : "memory"
    );
    return status_flag;
}

/**
 * @brief Loads a vector into a specified tile.
 * @param ctx The analog context managing the scales.
 * @param vec The vector to load into the tile.
 * @param tile_id The ID of the tile to load the vector.
 * @return The status flag indicating whether the operation was successful or not.
 */
template <typename T, typename qT = T>
uint16_t mvm_load_vector(AnalogContext &ctx, AnalogVector<T, qT> &vec, uint16_t tile_id) {
    vec.transfer_to_device(); // Transfer the vector to device (quantize if integral)

    ctx.set_input_vector(&vec, tile_id);

    void* data = vec.get_device_arr(); // Get the pointer to the device vector data
    uint16_t status_flag = 0;

    asm volatile (
        "mvm.l %0, %1, %2"
        : "=r" (status_flag)
        : "r"(data), "r"(tile_id)
        : "memory"
    );
    return status_flag;
}

/**
 * @brief Performs a computation on a specified tile.
 * @param ctx The analog context managing the scales.
 * @param tile_id The ID of the tile on which to perform the computation.
 * @return The status flag indicating whether the operation was successful or not.
 */
uint16_t mvm_compute(AnalogContext &ctx, uint16_t tile_id) {
    uint16_t status_flag = 0;

    asm volatile (
        "mvm %0, %1, x0"
        : "=r" (status_flag)
        : "r"(tile_id)
    );

    auto* mat = ctx.get_matrix(tile_id);
    auto* vec = ctx.get_input_vector(tile_id);
    ctx.compute_update(tile_id);
    vec->update_scale_factor(mat->get_scale_factor());
    return status_flag;
}

/**
 * @brief Stores a vector from a specified tile.
 * @param ctx The analog context managing the scales.
 * @param vec The vector to store from the tile.
 * @param tile_id The ID of the tile to store the vector from.
 * @return The status flag indicating whether the operation was successful or not.
 */
template <typename T, typename qT = T>
uint16_t mvm_store_vector(AnalogContext &ctx, AnalogVector<T, qT> &vec, uint16_t tile_id) {
    qT* data = vec.get_device_arr(); // Get the pointer to the device vector data
    uint16_t status_flag = 0;

    asm volatile (
        "mvm.s %0, %1, %2"
        : "=r" (status_flag)
        : "r"(data), "r"(tile_id)
        : "memory"
    );

    auto* input_vector = ctx.get_input_vector(tile_id); // Get the output scale for dequantization
    double scale = input_vector->get_scale_factor();
    vec.transfer_to_host(scale); // Transfer the vector to host (dequantize if integral)
    return status_flag;
}


uint32_t mvm_move_vector(AnalogContext &ctx, uint32_t tile_id, uint32_t tile_id_new) {
    uint32_t status_flag;

    ctx.move_vector(tile_id, tile_id_new);

    asm volatile (
        "mvm.mv %0, %1, %2"
        : "=r" (status_flag)
        : "r"(tile_id), "r"(tile_id_new)
    );

    return status_flag;
}

#endif // ANALOG_OPERATIONS_H

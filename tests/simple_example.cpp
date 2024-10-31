
int main() {
    // Dimensions for matrix and vector
    const int rows = 3;
    const int cols = 4;

    // Initialize matrix
    float** mat = new float*[rows];
    for (int i = 0; i < rows; i++) {
        mat[i] = new float[cols];
        for (int j = 0; j < cols; j++) {
            mat[i][j] = 3.0f;
        }
    }

    // Initialize vector
    float* vec = new float[cols];
    for (int i = 0; i < cols; i++) {
        vec[i] = 2.0f;
    }

    int status_flag;
    int tile_id = 0;

    asm volatile (
        "mvm.set %0, %1, %2"
      : "=r" (status_flag)
      : "r"(mat), "r"(tile_id)
      : "memory"
    );
}

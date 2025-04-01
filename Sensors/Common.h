// #ifndef __COMMON_H__
// #define __COMMON_H__
//     float fast_sqrt(float x);

//     //四元数
//     struct Quaternion{
//         float q0,q1,q2,q3;
//     };
//     //欧拉角
//     struct EulerAngle{
//         float pitch,roll,yaw;
//     };
//         struct EulerAngle QuaternionToEuler(struct Quaternion* e);
//     //矩阵计算辅助宏
//     #define MAT_MUL(AB, A, B, rows, inner, cols) \
//         for(i=0; i<rows; i++) \
//             for(j=0; j<cols; j++){ \
//                 AB[i][j] = 0; \
//                 for(k=0; k<inner; k++) \
//                     AB[i][j] += A[i][k] * B[k][j]; \
//             }\

//     //矩阵转置
//     #define MAT_TRANSPOSE(AT, A, rows, cols) \
//         for(i=0; i<cols; i++) \
//             for(j=0; j<rows; j++) \
//                 AT[i][j] = A[j][i];

//     // 添加矩阵相加的宏
//     #define MAT_ADD(C, A, B, rows, cols) \
//         for(i=0; i<rows; i++) \
//             for(j=0; j<cols; j++) \
//                 C[i][j] = A[i][j] + B[i][j];
    
//     // 添加矩阵相减的宏
//     #define MAT_SUB(C, A, B, rows, cols) \
//         for(i=0; i<rows; i++) \
//             for(j=0; j<cols; j++) \
//                 C[i][j] = A[i][j] - B[i][j];
                
//     // 添加矩阵初始化的宏
//     #define MAT_INIT(A, rows, cols, val) \
//         for(i=0; i<rows; i++) \
//             for(j=0; j<cols; j++) \
//                 A[i][j] = val;

// #endif
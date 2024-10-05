// robot_kinematics.c

// computeInverseKinematics function to return different integer codes
//    1: Solution found.
//    0: Maximum iterations reached without convergence.
//    -1: Pose unreachable within joint limits.
//    -2: Singular matrix encountered.
//    -3: Failed to converge (lambda too large)
// Enforce manufacturer's joint limits

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h> // For FLT_MAX

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Number of joints
#define NUM_JOINTS 6

// Threshold for convergence in inverse kinematics
#define IK_TOLERANCE 1e-6
#define IK_MAX_ITERATIONS 1000

// Structure to hold robot parameters
typedef struct {
    float a[NUM_JOINTS];           // Link lengths along x-axis (meters)
    float alpha[NUM_JOINTS];       // Link twists around x-axis (radians)
    float d[NUM_JOINTS];           // Link offsets along z-axis (meters)
    float theta_offset[NUM_JOINTS]; // Joint angle offsets (radians)
    float joint_min[NUM_JOINTS];   // Minimum joint angles (degrees)
    float joint_max[NUM_JOINTS];   // Maximum joint angles (degrees)
} RobotParameters;

// Function prototypes
void computeDHMatrix(float theta, float d, float a, float alpha, float T[4][4]);
void identityMatrix(float T[4][4]);
void multiplyMatrices(float A[4][4], float B[4][4], float result[4][4]);
void computeForwardKinematics(float jointAngles[NUM_JOINTS], float T0_6[4][4], RobotParameters *params);
int computeInverseKinematics(float desiredPose[6], float jointAngles[NUM_JOINTS], RobotParameters *params);
void printMatrix(float T[4][4]);
void printJointAngles(float jointAngles[NUM_JOINTS]);
void extractPoseFromMatrix(float T[4][4], float pose[6]);
void computeJacobian(float jointAngles[NUM_JOINTS], float J[6][NUM_JOINTS], RobotParameters *params);
int solveLinearSystem(float A[NUM_JOINTS][NUM_JOINTS], float b[NUM_JOINTS], float x[NUM_JOINTS], int n);

int main() {
    // Robot parameters with your provided DH parameters
    RobotParameters robotParams = {
        .a =      {0.0f, 0.0f, 0.135f, 0.038f, 0.0f, 0.0f}, // in meters
        .alpha =  {0.0f, -M_PI / 2, 0.0f, -M_PI / 2, M_PI / 2, -M_PI / 2}, // in radians
        .d =      {0.135f, 0.0f, 0.0f, 0.120f, 0.0f, 0.070f}, // in meters
        .theta_offset = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        .joint_min = {-170.0f, -120.0f, -170.0f, -120.0f, -170.0f, -120.0f}, // Degrees
        .joint_max = { 170.0f,  120.0f,  170.0f,  120.0f,  170.0f,  120.0f}  // Degrees
    };

    float jointAngles[NUM_JOINTS] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // Initial joint angles in degrees

    // Compute forward kinematics
    float T0_6[4][4];
    computeForwardKinematics(jointAngles, T0_6, &robotParams);

    printf("Forward Kinematics:\n");
    printMatrix(T0_6);

    // Desired end-effector pose (x, y, z in meters, roll, pitch, yaw in degrees)
    float desiredPose[6] = {0.3f, 0.2f, 0.5f, 0.0f, 90.0f, 0.0f};

    // Compute inverse kinematics
    int ikResult = computeInverseKinematics(desiredPose, jointAngles, &robotParams);
    if (ikResult == 1) {
        printf("Inverse Kinematics Solution Found:\n");
        printJointAngles(jointAngles);
    } else {
        printf("Inverse Kinematics Solution Not Found.\n");
        switch (ikResult) {
            case 0:
                printf("Reason: Maximum iterations reached without convergence.\n");
                break;
            case -1:
                printf("Reason: Pose unreachable within joint limits.\n");
                break;
            case -2:
                printf("Reason: Singular matrix encountered during computations.\n");
                break;
            case -3:
                printf("Reason: Failed to converge; damping factor became too large.\n");
                break;
            default:
                printf("Reason: Unknown error.\n");
                break;
        }
    }

    return 0;
}

// Function implementations

// Compute the DH transformation matrix for given parameters
void computeDHMatrix(float theta, float d, float a, float alpha, float T[4][4]) {
    // Angles are in radians
    float ct = cosf(theta);
    float st = sinf(theta);
    float ca = cosf(alpha);
    float sa = sinf(alpha);

    T[0][0] = ct;
    T[0][1] = -st * ca;
    T[0][2] = st * sa;
    T[0][3] = a * ct;

    T[1][0] = st;
    T[1][1] = ct * ca;
    T[1][2] = -ct * sa;
    T[1][3] = a * st;

    T[2][0] = 0.0f;
    T[2][1] = sa;
    T[2][2] = ca;
    T[2][3] = d;

    T[3][0] = 0.0f;
    T[3][1] = 0.0f;
    T[3][2] = 0.0f;
    T[3][3] = 1.0f;
}

// Initialize a 4x4 identity matrix
void identityMatrix(float T[4][4]) {
    memset(T, 0, sizeof(float) * 16);
    T[0][0] = 1.0f;
    T[1][1] = 1.0f;
    T[2][2] = 1.0f;
    T[3][3] = 1.0f;
}

// Multiply two 4x4 matrices A and B, store result in 'result'
void multiplyMatrices(float A[4][4], float B[4][4], float result[4][4]) {
    float temp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i][j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                temp[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    // Copy result back to result matrix
    memcpy(result, temp, sizeof(float) * 16);
}

// Compute the forward kinematics for the robot
void computeForwardKinematics(float jointAngles[NUM_JOINTS], float T0_6[4][4], RobotParameters *params) {
    identityMatrix(T0_6);

    for (int i = 0; i < NUM_JOINTS; i++) {
        // Convert joint angle to radians
        float theta = (jointAngles[i] + params->theta_offset[i]) * M_PI / 180.0f;
        float d = params->d[i];
        float a = params->a[i];
        float alpha = params->alpha[i]; // Already in radians

        float Ti[4][4];
        computeDHMatrix(theta, d, a, alpha, Ti);

        multiplyMatrices(T0_6, Ti, T0_6); // T0_6 = T0_6 * Ti
    }
}

// Print a 4x4 matrix
void printMatrix(float T[4][4]) {
    for (int i = 0; i < 4; i++) {
        printf("[ ");
        for (int j = 0; j < 4; j++) {
            printf("%8.4f ", T[i][j]);
        }
        printf("]\n");
    }
}

// Print joint angles in degrees
void printJointAngles(float jointAngles[NUM_JOINTS]) {
    for (int i = 0; i < NUM_JOINTS; i++) {
        printf("Joint %d: %8.4f degrees\n", i + 1, jointAngles[i]);
    }
}

// Extract position (x, y, z) and orientation (roll, pitch, yaw) from transformation matrix
void extractPoseFromMatrix(float T[4][4], float pose[6]) {
    // Position
    pose[0] = T[0][3];
    pose[1] = T[1][3];
    pose[2] = T[2][3];

    // Orientation (roll, pitch, yaw) in degrees
    float sy = sqrtf(T[0][0] * T[0][0] + T[1][0] * T[1][0]);

    if (sy > 1e-6) {
        pose[3] = atan2f(T[2][1], T[2][2]) * 180.0f / M_PI; // Roll
        pose[4] = atan2f(-T[2][0], sy) * 180.0f / M_PI;      // Pitch
        pose[5] = atan2f(T[1][0], T[0][0]) * 180.0f / M_PI;  // Yaw
    } else {
        pose[3] = atan2f(-T[1][2], T[1][1]) * 180.0f / M_PI; // Roll
        pose[4] = atan2f(-T[2][0], sy) * 180.0f / M_PI;      // Pitch
        pose[5] = 0.0f;                                       // Yaw
    }
}

// Compute the Jacobian matrix
void computeJacobian(float jointAngles[NUM_JOINTS], float J[6][NUM_JOINTS], RobotParameters *params) {
    float T0_i[NUM_JOINTS + 1][4][4]; // Transformation matrices from base to each joint frame
    float z[NUM_JOINTS + 1][3];       // z axes of each frame
    float p[NUM_JOINTS + 1][3];       // Origin positions of each frame

    // Initialize T0_0 as identity matrix
    identityMatrix(T0_i[0]);

    // Extract z[0] and p[0] from T0_i[0]
    z[0][0] = T0_i[0][0][2];
    z[0][1] = T0_i[0][1][2];
    z[0][2] = T0_i[0][2][2];
    p[0][0] = T0_i[0][0][3];
    p[0][1] = T0_i[0][1][3];
    p[0][2] = T0_i[0][2][3];

    // For each joint, compute T0_i and extract z_i and p_i
    for (int i = 0; i < NUM_JOINTS; i++) {
        // Convert joint angle to radians
        float theta = (jointAngles[i] + params->theta_offset[i]) * M_PI / 180.0f;
        float d = params->d[i];
        float a = params->a[i];
        float alpha = params->alpha[i];

        float Ti[4][4];
        computeDHMatrix(theta, d, a, alpha, Ti);

        // T0_i+1 = T0_i * Ti
        multiplyMatrices(T0_i[i], Ti, T0_i[i + 1]);

        // Extract z[i + 1] and p[i + 1] from T0_i+1
        z[i + 1][0] = T0_i[i + 1][0][2];
        z[i + 1][1] = T0_i[i + 1][1][2];
        z[i + 1][2] = T0_i[i + 1][2][2];

        p[i + 1][0] = T0_i[i + 1][0][3];
        p[i + 1][1] = T0_i[i + 1][1][3];
        p[i + 1][2] = T0_i[i + 1][2][3];
    }

    // Compute p_n (position of end effector)
    float p_n[3];
    p_n[0] = p[NUM_JOINTS][0];
    p_n[1] = p[NUM_JOINTS][1];
    p_n[2] = p[NUM_JOINTS][2];

    // Now compute Jacobian columns
    for (int i = 0; i < NUM_JOINTS; i++) {
        float Jv[3];
        float Jw[3];

        // z_i = z[i]
        // p_i = p[i]
        // Jv_i = z_i x (p_n - p_i)
        float p_diff[3] = {p_n[0] - p[i][0], p_n[1] - p[i][1], p_n[2] - p[i][2]};
        // Cross product z[i] x p_diff
        Jv[0] = z[i][1] * p_diff[2] - z[i][2] * p_diff[1];
        Jv[1] = z[i][2] * p_diff[0] - z[i][0] * p_diff[2];
        Jv[2] = z[i][0] * p_diff[1] - z[i][1] * p_diff[0];

        // Jw_i = z_i
        Jw[0] = z[i][0];
        Jw[1] = z[i][1];
        Jw[2] = z[i][2];

        // Fill Jacobian matrix
        J[0][i] = Jv[0];
        J[1][i] = Jv[1];
        J[2][i] = Jv[2];
        J[3][i] = Jw[0];
        J[4][i] = Jw[1];
        J[5][i] = Jw[2];
    }
}

// Solve a linear system A * x = b using Gaussian elimination
int solveLinearSystem(float A[NUM_JOINTS][NUM_JOINTS], float b[NUM_JOINTS], float x[NUM_JOINTS], int n) {
    // Copy A and b to local variables since we'll modify them
    float a[NUM_JOINTS][NUM_JOINTS];
    float y[NUM_JOINTS];
    for (int i = 0; i < n; i++) {
        y[i] = b[i];
        for (int j = 0; j < n; j++) {
            a[i][j] = A[i][j];
        }
    }

    // Forward elimination
    for (int k = 0; k < n; k++) {
        // Find pivot
        float max = fabsf(a[k][k]);
        int maxRow = k;
        for (int i = k + 1; i < n; i++) {
            if (fabsf(a[i][k]) > max) {
                max = fabsf(a[i][k]);
                maxRow = i;
            }
        }

        // Swap rows if needed
        if (maxRow != k) {
            for (int j = 0; j < n; j++) {
                float tmp = a[k][j];
                a[k][j] = a[maxRow][j];
                a[maxRow][j] = tmp;
            }
            float tmp = y[k];
            y[k] = y[maxRow];
            y[maxRow] = tmp;
        }

        // Check for zero pivot
        if (fabsf(a[k][k]) < 1e-12) {
            return 0; // Singular matrix
        }

        // Eliminate below
        for (int i = k + 1; i < n; i++) {
            float factor = a[i][k] / a[k][k];
            for (int j = k; j < n; j++) {
                a[i][j] -= factor * a[k][j];
            }
            y[i] -= factor * y[k];
        }
    }

    // Back substitution
    for (int i = n - 1; i >= 0; i--) {
        x[i] = y[i];
        for (int j = i + 1; j < n; j++) {
            x[i] -= a[i][j] * x[j];
        }
        x[i] /= a[i][i];
    }

    return 1; // Success
}

// Inverse kinematics using Levenberg-Marquardt algorithm with joint limits and dynamic damping factor
int computeInverseKinematics(float desiredPose[6], float jointAngles[NUM_JOINTS], RobotParameters *params) {
    float lambda = 0.01f; // Initial damping factor
    float nu = 2.0f;      // Factor to adjust lambda
    float deltaTheta[NUM_JOINTS];
    float currentPose[6];
    float error[6];
    float J[6][NUM_JOINTS];
    float J_T[NUM_JOINTS][6];
    float A[NUM_JOINTS][NUM_JOINTS];
    float b[NUM_JOINTS];

    for (int iter = 0; iter < IK_MAX_ITERATIONS; iter++) {
        // Compute current end-effector pose
        float T0_6[4][4];
        computeForwardKinematics(jointAngles, T0_6, params);
        extractPoseFromMatrix(T0_6, currentPose);

        // Compute error between desired and current pose
        for (int i = 0; i < 6; i++) {
            error[i] = desiredPose[i] - currentPose[i];
            // Normalize angle differences to [-180, 180]
            if (i >= 3) {
                while (error[i] > 180.0f) error[i] -= 360.0f;
                while (error[i] < -180.0f) error[i] += 360.0f;
                // Convert to radians
                error[i] = error[i] * M_PI / 180.0f;
            }
        }

        // Compute error norm
        float errorNorm = 0.0f;
        for (int i = 0; i < 6; i++) {
            errorNorm += error[i] * error[i];
        }
        errorNorm = sqrtf(errorNorm);

        // Check for convergence
        if (errorNorm < IK_TOLERANCE) {
            return 1; // Solution found
        }

        // Compute Jacobian
        computeJacobian(jointAngles, J, params);

        // Compute J^T
        for (int i = 0; i < NUM_JOINTS; i++) {
            for (int j = 0; j < 6; j++) {
                J_T[i][j] = J[j][i];
            }
        }

        // Compute A = J^T * J + lambda * I
        for (int i = 0; i < NUM_JOINTS; i++) {
            for (int j = 0; j < NUM_JOINTS; j++) {
                A[i][j] = 0.0f;
                for (int k = 0; k < 6; k++) {
                    A[i][j] += J_T[i][k] * J[k][j];
                }
                if (i == j) {
                    A[i][j] += lambda;
                }
            }
        }

        // Compute b = J^T * error
        for (int i = 0; i < NUM_JOINTS; i++) {
            b[i] = 0.0f;
            for (int k = 0; k < 6; k++) {
                b[i] += J_T[i][k] * error[k];
            }
        }

        // Solve A * deltaTheta = b
        if (!solveLinearSystem(A, b, deltaTheta, NUM_JOINTS)) {
            // Cannot solve linear system
            return -2; // Singular matrix encountered
        }

        // Tentatively update joint angles
        float tempJointAngles[NUM_JOINTS];
        for (int i = 0; i < NUM_JOINTS; i++) {
            tempJointAngles[i] = jointAngles[i] + deltaTheta[i] * 180.0f / M_PI;
        }

        // Enforce joint limits
        int withinLimits = 1;
        for (int i = 0; i < NUM_JOINTS; i++) {
            if (tempJointAngles[i] > params->joint_max[i] || tempJointAngles[i] < params->joint_min[i]) {
                withinLimits = 0;
                break;
            }
        }
        if (!withinLimits) {
            return -1; // Pose unreachable within joint limits
        }

        // Compute new error norm with tentative joint angles
        float T0_6_new[4][4];
        computeForwardKinematics(tempJointAngles, T0_6_new, params);
        float newPose[6];
        extractPoseFromMatrix(T0_6_new, newPose);

        // Compute new error norm
        float newError[6];
        float newErrorNorm = 0.0f;
        for (int i = 0; i < 6; i++) {
            newError[i] = desiredPose[i] - newPose[i];
            if (i >= 3) {
                while (newError[i] > 180.0f) newError[i] -= 360.0f;
                while (newError[i] < -180.0f) newError[i] += 360.0f;
                newError[i] = newError[i] * M_PI / 180.0f; // Convert to radians
            }
            newErrorNorm += newError[i] * newError[i];
        }
        newErrorNorm = sqrtf(newErrorNorm);

        // Adjust lambda based on error norm
        if (newErrorNorm < errorNorm) {
            // Error decreased; accept the update
            for (int i = 0; i < NUM_JOINTS; i++) {
                jointAngles[i] = tempJointAngles[i];
            }
            lambda /= nu; // Decrease lambda
            errorNorm = newErrorNorm;
        } else {
            // Error increased; reject the update
            lambda *= nu; // Increase lambda
            if (lambda > 1e7f) {
                // Lambda too large; convergence unlikely
                return -3; // Failed to converge
            }
        }
    }

    return 0; // Maximum iterations reached without convergence
}

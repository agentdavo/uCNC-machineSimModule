# Robot Kinematics Module

This README provides an overview of the `robot_kinematics.c` module, which implements forward and inverse kinematics for a 6-DOF robotic arm, such as the Meca500. The module includes functions to compute transformation matrices, the Jacobian matrix, and to solve inverse kinematics using the Levenberg-Marquardt algorithm with joint limits enforcement and dynamic damping factor adjustment.

---

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Compilation and Execution](#compilation-and-execution)
- [Usage](#usage)
  - [Function Overview](#function-overview)
  - [Examples](#examples)
- [Error Codes](#error-codes)
- [Limitations and Notes](#limitations-and-notes)
- [License](#license)

---

## Introduction

The `robot_kinematics.c` module provides essential kinematic calculations for robotic manipulators. It includes:

- **Forward Kinematics**: Computes the end-effector pose given joint angles.
- **Inverse Kinematics**: Calculates joint angles required to achieve a desired end-effector pose.
- **Jacobian Computation**: Determines the Jacobian matrix for velocity and force calculations.
- **Joint Limits Enforcement**: Ensures joint angles stay within specified limits.
- **Dynamic Damping Factor**: Adjusts the damping factor in the inverse kinematics algorithm for improved convergence.

---

## Features

- Implements Denavit-Hartenberg (DH) parameters for the robot's kinematic model.
- Uses the Levenberg-Marquardt algorithm for inverse kinematics.
- Enforces manufacturer-specified joint limits.
- Adjusts damping factor dynamically to enhance convergence.
- Provides detailed error codes for robust error handling.

---

## Prerequisites

- **C Compiler**: GCC or any standard C compiler that supports C99 or later.
- **Math Library**: The standard C math library (`-lm` flag during compilation).

---

## Compilation and Execution

To compile the `robot_kinematics.c` module and the example `main` function provided:

```bash
gcc robot_kinematics.c -o robot_kinematics -lm
```

To run the program:

```bash
./robot_kinematics
```

---

## Usage

### Function Overview

#### 1. **computeDHMatrix**

```c
void computeDHMatrix(float theta, float d, float a, float alpha, float T[4][4]);
```

- Computes the Denavit-Hartenberg transformation matrix for given parameters.

#### 2. **identityMatrix**

```c
void identityMatrix(float T[4][4]);
```

- Initializes a 4x4 identity matrix.

#### 3. **multiplyMatrices**

```c
void multiplyMatrices(float A[4][4], float B[4][4], float result[4][4]);
```

- Multiplies two 4x4 matrices.

#### 4. **computeForwardKinematics**

```c
void computeForwardKinematics(float jointAngles[NUM_JOINTS], float T0_6[4][4], RobotParameters *params);
```

- Computes the forward kinematics, yielding the end-effector transformation matrix.

#### 5. **computeInverseKinematics**

```c
int computeInverseKinematics(float desiredPose[6], float jointAngles[NUM_JOINTS], RobotParameters *params);
```

- Solves for joint angles given a desired end-effector pose.

#### 6. **computeJacobian**

```c
void computeJacobian(float jointAngles[NUM_JOINTS], float J[6][NUM_JOINTS], RobotParameters *params);
```

- Computes the Jacobian matrix for the current joint configuration.

#### 7. **solveLinearSystem**

```c
int solveLinearSystem(float A[NUM_JOINTS][NUM_JOINTS], float b[NUM_JOINTS], float x[NUM_JOINTS], int n);
```

- Solves a linear system \( A \mathbf{x} = \mathbf{b} \) using Gaussian elimination.

#### 8. **extractPoseFromMatrix**

```c
void extractPoseFromMatrix(float T[4][4], float pose[6]);
```

- Extracts position and orientation from a transformation matrix.

#### 9. **printMatrix**

```c
void printMatrix(float T[4][4]);
```

- Prints a 4x4 matrix to the console.

#### 10. **printJointAngles**

```c
void printJointAngles(float jointAngles[NUM_JOINTS]);
```

- Prints joint angles in degrees.

### Examples

#### 1. **Computing Forward Kinematics**

```c
// Define joint angles in degrees
float jointAngles[NUM_JOINTS] = {0.0f, -45.0f, 30.0f, 0.0f, 90.0f, 0.0f};

// Compute forward kinematics
float T0_6[4][4];
computeForwardKinematics(jointAngles, T0_6, &robotParams);

// Print the transformation matrix
printf("End-Effector Transformation Matrix:\n");
printMatrix(T0_6);

// Extract and print the pose
float pose[6];
extractPoseFromMatrix(T0_6, pose);
printf("End-Effector Pose:\n");
printf("Position - X: %f, Y: %f, Z: %f\n", pose[0], pose[1], pose[2]);
printf("Orientation - Roll: %f, Pitch: %f, Yaw: %f\n", pose[3], pose[4], pose[5]);
```

#### 2. **Computing Inverse Kinematics**

```c
// Desired end-effector pose
float desiredPose[6] = {0.3f, 0.2f, 0.5f, 0.0f, 90.0f, 0.0f};

// Initial guess for joint angles
float jointAngles[NUM_JOINTS] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

// Compute inverse kinematics
int ikResult = computeInverseKinematics(desiredPose, jointAngles, &robotParams);

if (ikResult == 1) {
    printf("Inverse Kinematics Solution Found:\n");
    printJointAngles(jointAngles);
} else {
    printf("Inverse Kinematics Solution Not Found.\n");
    // Handle errors based on the error code
}
```

---

## Error Codes

The `computeInverseKinematics` function returns specific integer codes to indicate the outcome:

- **1**: Solution found successfully.
- **0**: Maximum iterations reached without convergence.
- **-1**: Pose unreachable within joint limits.
- **-2**: Singular matrix encountered during computations.
- **-3**: Failed to converge; damping factor became too large.

---

## Limitations and Notes

- **Joint Limits**: The module enforces joint limits specified in the `RobotParameters` structure. Ensure these limits match the robot's specifications.

- **Convergence**: The inverse kinematics algorithm may not converge for poses outside the robot's workspace or near singularities.

- **Units Consistency**: Angles are in degrees for input/output convenience but are converted to radians internally for calculations. Distances are in meters.

- **Error Handling**: Proper error handling is essential when the inverse kinematics function does not find a solution. Use the returned error codes to determine the appropriate response.

- **Numerical Stability**: The `solveLinearSystem` function uses Gaussian elimination without partial pivoting. For large systems or systems prone to numerical instability, consider using a more robust linear solver.

- **Performance**: The code is intended for educational purposes and may not be optimized for real-time applications. Optimization may be necessary for deployment on embedded systems.

---

## License

This code is provided under the [MIT License](LICENSE). You are free to use, modify, and distribute it as per the license terms.

---
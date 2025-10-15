#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "emergency_module.h"

// Test statistics
static int tests_passed = 0;
static int tests_failed = 0;

// Test result tracking
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  ❌ FAILED: %s\n", message); \
            tests_failed++; \
            return; \
        } \
    } while(0)

#define TEST_PASS(name) \
    do { \
        printf("  ✅ PASSED: %s\n", name); \
        tests_passed++; \
    } while(0)

// RIGHT-BICEP Testing Paradigm Implementation
// R - Right: Are the results right?
// I - Inverse: Can you check inverse relationships?
// C - Cross-check: Can you cross-check results using other means?
// E - Error conditions: Can you force error conditions?
// P - Performance: Is it fast enough, scalable?

// ====================
// BASIC FUNCTIONALITY TESTS
// ====================

void test_class_init_right() {
    printf("\n[RIGHT] Testing EmergencyNode_class_init basic functionality...\n");
    
    int8_t result = EmergencyNode_class_init();
    TEST_ASSERT(result == 0, "First initialization should succeed");
    
    result = EmergencyNode_class_init();
    TEST_ASSERT(result == -1, "Second initialization should fail");
    
    TEST_PASS("Class initialization");
}

void test_node_init_right() {
    printf("\n[RIGHT] Testing EmergencyNode_init...\n");
    
    EmergencyNode_t node;
    memset(&node, 0xFF, sizeof(node)); // Fill with garbage
    
    int8_t result = EmergencyNode_init(&node);
    TEST_ASSERT(result == 0, "Init should return 0");
    TEST_ASSERT(node.emergency_counter == 0, "Counter should be zeroed");
    
    for (int i = 0; i < NUM_EMERGENCY_BUFFER; i++) {
        TEST_ASSERT(node.emergency_buffer[i] == 0, "Buffer should be zeroed");
    }
    
    TEST_PASS("Node initialization");
}

void test_raise_emergency_right() {
    printf("\n[RIGHT] Testing EmergencyNode_raise...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    // Raise emergency 5
    int8_t result = EmergencyNode_raise(&node, 5);
    TEST_ASSERT(result == 0, "Raise should succeed");
    TEST_ASSERT(node.emergency_counter == 1, "Counter should be 1");
    TEST_ASSERT((node.emergency_buffer[0] & (1 << 5)) != 0, "Bit 5 should be set");
    
    // Raise same emergency again (idempotent)
    result = EmergencyNode_raise(&node, 5);
    TEST_ASSERT(result == 0, "Raise should succeed");
    TEST_ASSERT(node.emergency_counter == 1, "Counter should still be 1");
    
    // Raise different emergency
    result = EmergencyNode_raise(&node, 10);
    TEST_ASSERT(result == 0, "Raise should succeed");
    TEST_ASSERT(node.emergency_counter == 2, "Counter should be 2");
    
    TEST_PASS("Emergency raise");
}

void test_solve_emergency_right() {
    printf("\n[RIGHT] Testing EmergencyNode_solve...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    EmergencyNode_raise(&node, 5);
    EmergencyNode_raise(&node, 10);
    TEST_ASSERT(node.emergency_counter == 2, "Counter should be 2");
    
    int8_t result = EmergencyNode_solve(&node, 5);
    TEST_ASSERT(result == 0, "Solve should succeed");
    TEST_ASSERT(node.emergency_counter == 1, "Counter should be 1");
    TEST_ASSERT((node.emergency_buffer[0] & (1 << 5)) == 0, "Bit 5 should be cleared");
    
    result = EmergencyNode_solve(&node, 10);
    TEST_ASSERT(result == 0, "Solve should succeed");
    TEST_ASSERT(node.emergency_counter == 0, "Counter should be 0");
    
    TEST_PASS("Emergency solve");
}

// ====================
// INVERSE RELATIONSHIP TESTS
// ====================

void test_raise_solve_inverse() {
    printf("\n[INVERSE] Testing raise/solve inverse relationship...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    // Raise and solve multiple emergencies
    for (uint8_t i = 0; i < 20; i++) {
        EmergencyNode_raise(&node, i);
    }
    TEST_ASSERT(node.emergency_counter == 20, "Counter should be 20");
    
    for (uint8_t i = 0; i < 20; i++) {
        EmergencyNode_solve(&node, i);
    }
    TEST_ASSERT(node.emergency_counter == 0, "Counter should return to 0");
    
    TEST_PASS("Raise/solve inverse relationship");
}

// ====================
// CROSS-CHECK TESTS
// ====================

void test_emergency_state_cross_check() {
    printf("\n[CROSS-CHECK] Testing emergency state detection...\n");

    EmergencyNode_t node;
    EmergencyNode_init(&node);

    // Verify that node counter is 0
    TEST_ASSERT(node.emergency_counter == 0, "Node counter should be 0 initially");

    EmergencyNode_raise(&node, 7);
    int8_t state = EmergencyNode_is_emergency_state(&node);
    TEST_ASSERT(state != 0, "Should be in emergency state after raise");
    TEST_ASSERT(node.emergency_counter == 1, "Node counter should be 1 after raise");

    EmergencyNode_solve(&node, 7);
    TEST_ASSERT(node.emergency_counter == 0, "Node counter should be 0 after solve");

    TEST_PASS("Emergency state cross-check");
}

// ====================
// ERROR CONDITION TESTS
// ====================

void test_boundary_conditions_error() {
    printf("\n[ERROR] Testing boundary conditions...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    // Test maximum valid emergency ID (NUM_EMERGENCY_BUFFER * 8 - 1 = 63)
    int8_t result = EmergencyNode_raise(&node, 63);
    TEST_ASSERT(result == 0, "Max valid ID should succeed");
    
    // Test out of bounds
    result = EmergencyNode_raise(&node, 64);
    TEST_ASSERT(result == -1, "ID 64 should fail (out of bounds)");
    
    result = EmergencyNode_solve(&node, 64);
    TEST_ASSERT(result == -1, "Solve ID 64 should fail");
    
    TEST_PASS("Boundary condition handling");
}

void test_solve_nonexistent_error() {
    printf("\n[ERROR] Testing solve non-existent emergency...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    // Solving emergency that was never raised should be safe
    int8_t result = EmergencyNode_solve(&node, 5);
    TEST_ASSERT(result == 0, "Solving non-existent should succeed gracefully");
    TEST_ASSERT(node.emergency_counter == 0, "Counter should remain 0");
    
    TEST_PASS("Solve non-existent emergency");
}

void test_destroy_with_active_emergencies() {
    printf("\n[ERROR] Testing destroy with active emergencies...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    EmergencyNode_raise(&node, 5);
    EmergencyNode_raise(&node, 10);
    TEST_ASSERT(node.emergency_counter == 2, "Counter should be 2");
    
    int8_t result = EmergencyNode_destroy(&node);
    TEST_ASSERT(result == 0, "Destroy should succeed");
    TEST_ASSERT(node.emergency_counter == 0, "Counter should be cleared");
    
    TEST_PASS("Destroy with active emergencies");
}

// ====================
// PERFORMANCE TESTS
// ====================

void test_performance_many_operations() {
    printf("\n[PERFORMANCE] Testing many sequential operations...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    // Perform 10000 operations
    for (int i = 0; i < 10000; i++) {
        uint8_t id = i % 64;
        EmergencyNode_raise(&node, id);
    }
    
    for (int i = 0; i < 10000; i++) {
        uint8_t id = i % 64;
        EmergencyNode_solve(&node, id);
    }
    
    TEST_ASSERT(node.emergency_counter == 0, "All emergencies should be solved");
    
    TEST_PASS("Performance with many operations");
}

// ====================
// MULTITHREADED TESTS
// ====================

typedef struct {
    EmergencyNode_t* node;
    int thread_id;
    int iterations;
} ThreadTestData;

void* thread_raise_worker(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->iterations; i++) {
        uint8_t emergency_id = (data->thread_id * 8 + (i % 8)) % 64;
        EmergencyNode_raise(data->node, emergency_id);
        usleep(10); // Small delay to increase contention
    }
    
    return NULL;
}

void* thread_solve_worker(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->iterations; i++) {
        uint8_t emergency_id = (data->thread_id * 8 + (i % 8)) % 64;
        EmergencyNode_solve(data->node, emergency_id);
        usleep(10);
    }
    
    return NULL;
}

void test_multithreaded_raise() {
    printf("\n[MULTITHREADED] Testing concurrent raise operations...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    const int NUM_THREADS = 4;
    const int ITERATIONS = 100;
    pthread_t threads[NUM_THREADS];
    ThreadTestData thread_data[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].node = &node;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = ITERATIONS;
        pthread_create(&threads[i], NULL, thread_raise_worker, &thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    TEST_ASSERT(node.emergency_counter > 0, "Some emergencies should be raised");
    TEST_ASSERT(node.emergency_counter <= 64, "Counter should not exceed max emergencies");
    
    TEST_PASS("Multithreaded raise operations");
}

void test_multithreaded_raise_and_solve() {
    printf("\n[MULTITHREADED] Testing concurrent raise and solve...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    const int NUM_THREADS = 8;
    const int ITERATIONS = 50;
    pthread_t threads[NUM_THREADS];
    ThreadTestData thread_data[NUM_THREADS];
    
    // Half threads raise, half solve
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].node = &node;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = ITERATIONS;
        
        if (i < NUM_THREADS / 2) {
            pthread_create(&threads[i], NULL, thread_raise_worker, &thread_data[i]);
        } else {
            pthread_create(&threads[i], NULL, thread_solve_worker, &thread_data[i]);
        }
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // System should remain consistent
    TEST_ASSERT(node.emergency_counter <= 64, "Counter should be valid");
    
    TEST_PASS("Multithreaded raise and solve");
}

void* thread_stress_worker(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->iterations; i++) {
        uint8_t emergency_id = rand() % 64;
        
        if (rand() % 2) {
            EmergencyNode_raise(data->node, emergency_id);
        } else {
            EmergencyNode_solve(data->node, emergency_id);
        }
    }
    
    return NULL;
}

void test_multithreaded_stress() {
    printf("\n[MULTITHREADED] Stress testing with random operations...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    const int NUM_THREADS = 10;
    const int ITERATIONS = 1000;
    pthread_t threads[NUM_THREADS];
    ThreadTestData thread_data[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].node = &node;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = ITERATIONS;
        pthread_create(&threads[i], NULL, thread_stress_worker, &thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    TEST_ASSERT(node.emergency_counter <= 64, "Counter should remain valid after stress");
    
    TEST_PASS("Multithreaded stress test");
}

// ====================
// ADDITIONAL EDGE CASES
// ====================

void test_all_emergencies_simultaneously() {
    printf("\n[EDGE CASE] Testing all 64 emergencies simultaneously...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    // Raise all possible emergencies
    for (uint8_t i = 0; i < 64; i++) {
        EmergencyNode_raise(&node, i);
    }
    
    TEST_ASSERT(node.emergency_counter == 64, "All 64 emergencies should be active");
    
    // Verify all bits are set
    for (int i = 0; i < NUM_EMERGENCY_BUFFER; i++) {
        TEST_ASSERT(node.emergency_buffer[i] == 0xFF, "All bits in buffer should be set");
    }
    
    // Solve all
    for (uint8_t i = 0; i < 64; i++) {
        EmergencyNode_solve(&node, i);
    }
    
    TEST_ASSERT(node.emergency_counter == 0, "All emergencies should be solved");
    
    TEST_PASS("All emergencies simultaneously");
}

void test_byte_boundary_emergencies() {
    printf("\n[EDGE CASE] Testing emergencies across byte boundaries...\n");
    
    EmergencyNode_t node;
    EmergencyNode_init(&node);
    
    // Test boundaries: 7, 8, 15, 16, 23, 24, etc.
    uint8_t boundaries[] = {7, 8, 15, 16, 23, 24, 31, 32, 39, 40, 47, 48, 55, 56, 63};
    
    for (int i = 0; i < sizeof(boundaries); i++) {
        EmergencyNode_raise(&node, boundaries[i]);
    }
    
    TEST_ASSERT(node.emergency_counter == sizeof(boundaries), "All boundary emergencies should be raised");
    
    for (int i = 0; i < sizeof(boundaries); i++) {
        EmergencyNode_solve(&node, boundaries[i]);
    }
    
    TEST_ASSERT(node.emergency_counter == 0, "All boundary emergencies should be solved");
    
    TEST_PASS("Byte boundary emergencies");
}

// ====================
// MAIN TEST RUNNER
// ====================

int main(void) {
    printf("=================================================\n");
    printf("  Emergency Module Unit Test Suite\n");
    printf("  Using RIGHT-BICEP Testing Paradigm\n");
    printf("=================================================\n");
    
    // Initialize the emergency system
    // EmergencyNode_class_init();
    
    // Run all tests
    test_class_init_right();
    test_node_init_right();
    test_raise_emergency_right();
    test_solve_emergency_right();
    
    test_raise_solve_inverse();
    
    test_emergency_state_cross_check(); // FAILS
    
    test_boundary_conditions_error();
    test_solve_nonexistent_error();
    test_destroy_with_active_emergencies();
    
    test_performance_many_operations();
    
    test_multithreaded_raise();
    test_multithreaded_raise_and_solve();
    test_multithreaded_stress();
    
    test_all_emergencies_simultaneously();
    test_byte_boundary_emergencies();
    
    // Print summary
    printf("\n=================================================\n");
    printf("  Test Results Summary\n");
    printf("=================================================\n");
    printf("  ✅ Passed: %d\n", tests_passed);
    printf("  ❌ Failed: %d\n", tests_failed);
    printf("  Total:  %d\n", tests_passed + tests_failed);
    printf("=================================================\n");
    
    return tests_failed > 0 ? 1 : 0;
}

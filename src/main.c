#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/pm/policy.h>
#include <zephyr/pm/pm.h>

#define SLEEP_TIME_MS 5000


 //This function is called by Zephyr's PM subsystem whenever the CPU becomes idle.
 
 //PM Policy callback
const struct pm_state_info *pm_policy_next_state(uint8_t cpu, int32_t ticks)
{
    ARG_UNUSED(ticks);   //Prevents unused variable warnings 

    printk("\n>>> pm_policy_next_state() CALLED <<<\n");

    const struct pm_state_info *cpu_states;      // Pointer to available PM states
    uint8_t num_states;                          // Number of available states

    num_states = pm_state_cpu_get_all(cpu, &cpu_states);   // Get all supported power states for this CPU

    printk("Number of CPU power states: %d\n", num_states);

    if (num_states == 0) {
        printk("ERROR: No power states found!\n");
        return NULL;   //No state available
    }

// Loop through all available states 
    for (int i = 0; i < num_states; i++) {

        printk("Checking state %d (enum value = %d)\n",
               i,
               cpu_states[i].state);

        if (cpu_states[i].state == PM_STATE_RUNTIME_IDLE) {

            printk("Selecting PM_STATE_RUNTIME_IDLE\n");

            return &cpu_states[i];     //tells zephyr to enter this state
        }
    }

    printk("ERROR: PM_STATE_RUNTIME_IDLE not found!\n");

    return NULL;   //No matching state found
}

int main(void)
{
    printk("\n");
    printk("=====================================\n");
    printk(" MCXN947 Sleep Mode Test\n");
    printk("=====================================\n");

    while (1) {

        printk("\nCPU ACTIVE\n");                   //CPU running normally 
        printk("Preparing to enter Sleep...\n");

        k_sleep(K_MSEC(SLEEP_TIME_MS));   // Allows idle mode 

        printk("CPU AWAKE\n");    //Runs after sleep completes
    }

    return 0;
}

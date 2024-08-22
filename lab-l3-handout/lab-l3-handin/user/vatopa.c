#include "kernel/types.h"
#include "user.h"

// skal kalle på syscallet (tar inn en virtuell adresse og en pid som argument)
// metoden sjekker om antall argumenter er riktig og henter opp va, pid og kaller va2pa for å få fysisk adresse

// sjekker om det er ett argument gitt (eks. "vatopa") og printer teksten 
int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Usage: vatopa virtual_address [pid]\n");
        return 1; // Return an error code
    }
    
    //setter pid til programmets egen pid som standardverdi 
    uint64 virtual_address = atoi(argv[1]);
    int pid = getpid();
    if (argc == 3) {
        pid = atoi(argv[2]);
    }

    // kaller på va2pa systemkallet, oversetter va til fysisk 
    uint64 physical_address = va2pa(virtual_address, pid);

    // sjekker om den finner gyldig adresse
    if (physical_address == 0) {
        printf("Error: Failed to translate va to physical\n");
        return 1; // Return an error code
    }

    printf("0x%x\n", physical_address);
    return 0;
}

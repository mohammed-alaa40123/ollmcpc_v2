#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <grp.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sched.h>
#include <errno.h>

#define AUDIT_LOG_FILE "system_audit.log"
#define ADMIN_GROUP "sudo" 


typedef enum {
    LVL_USER = 0,     
    LVL_ADMIN = 1,    
    LVL_DANGEROUS = 2 
} PrivilegeLevel;


typedef struct {
    char *name;
    PrivilegeLevel level;
    char *warning_msg; 
    int is_internal;   
    void (*internal_func)(int argc, char **argv);
} CommandMetadata;


void cmd_sched(int argc, char **argv);
void cmd_wipe(int argc, char **argv);


CommandMetadata internal_cmds[] = {

    {"sched", LVL_ADMIN, NULL, 1, cmd_sched},
    {"wipe",  LVL_DANGEROUS, "This clears system caches and logs.", 1, cmd_wipe},
    {NULL, 0, NULL, 0, NULL}
};


const char *dangerous_binaries[] = {
    "rm","rmdir", "dd", "mkfs", "fdisk", "reboot", "shutdown", "chmod", "chown","run_shell_command.sh", NULL
};



void log_audit(const char *cmd_name, const char *user, const char *status, const char *details) {
    FILE *fp = fopen(AUDIT_LOG_FILE, "a");
    if (fp) {
        time_t now;
        time(&now);
        char *date = ctime(&now);
        date[strlen(date) - 1] = '\0';
        fprintf(fp, "[%s] USER:%s CMD:%s STATUS:%s MSG:%s\n", date, user, cmd_name, status, details);
        fclose(fp);
    }
}

int is_authorized_user() {
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (!pw) return 0;
    if (uid == 0) return 1; // Root is always auth

    int ngroups = 0;
    getgrouplist(pw->pw_name, pw->pw_gid, NULL, &ngroups);
    gid_t *groups = malloc(ngroups * sizeof(gid_t));
    getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);

    struct group *gr = getgrnam(ADMIN_GROUP);
    if (!gr) { free(groups); return 0; }

    int auth = 0;
    for (int i = 0; i < ngroups; i++) {
        if (groups[i] == gr->gr_gid) { auth = 1; break; }
    }
    free(groups);
    return auth;
}

void cmd_sched(int argc, char **argv) {
    if (argc < 3) {
        printf("[KERNEL] Error: Missing PID. Usage: sched <pid>\n");
        return;
    }

    int pid = atoi(argv[2]); // The shell passes: kernel_dispatcher sched <pid>
    int policy = SCHED_RR;   // Real-time Round Robin policy
    struct sched_param param;

    // Set priority to the maximum allowed for Round Robin
    param.sched_priority = sched_get_priority_max(policy);

    printf("[KERNEL] Attempting to set PID %d to Real-Time Round Robin policy...\n", pid);

    // SYSTEM CALL: sched_setscheduler
    if (sched_setscheduler(pid, policy, &param) == -1) {
        // Handle Errors (Usually EPERM if not root)
        perror("[KERNEL] System Call Failed"); 
        if (errno == EPERM) {
            printf("[KERNEL] Hint: The Protection Boundary requires ROOT for scheduler changes.\n");
        }
    } else {
        printf("[KERNEL] Success! PID %d is now running with SCHED_RR priority %d.\n", 
               pid, param.sched_priority);
        printf("[KERNEL] Verification: Run 'chrt -p %d' to check.\n", pid);
    }
}

void cmd_wipe(int argc, char **argv) {
    printf("[KERNEL] Syncing filesystem buffers to disk...\n");
    
    // 1. Sync data to disk first to prevent data loss
    sync(); 

    printf("[KERNEL] Writing to /proc/sys/vm/drop_caches...\n");

    // 2. Open the kernel tunable file
    FILE *fp = fopen("/proc/sys/vm/drop_caches", "w");
    if (fp == NULL) {
        perror("[KERNEL] Failed to open /proc/sys/vm/drop_caches");
        if (errno == EACCES || errno == EPERM) {
            printf("[KERNEL] Hint: Only the superuser (Root) can drop caches.\n");
        }
        return;
    }

    // 3. Write '3' (Clear PageCache, dentries, and inodes)
    if (fprintf(fp, "3\n") < 0) {
        perror("[KERNEL] Failed to write to kernel interface");
    } else {
        printf("[KERNEL] Success: System PageCache, dentries, and inodes cleared.\n");
        printf("[KERNEL] Free memory should now be higher. Check with 'free -h'.\n");
    }

    fclose(fp);
}

// --- Main Logic ---

int main(int argc, char **argv) {
    // 1. Validation: Ensure exactly one argument is passed
    // if (argc < 2) {
    //     fprintf(stderr, "Usage: %s \"string to split\"\n", argv[0]);
    //     return 1;
    // }

    printf("--- Before Modification ---\n");
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    // 2. Duplicate the input string
    // We do this because strtok modifies the string in-place (adding null terminators).
    // We want a heap-allocated copy so we don't mess with the original stack/env memory.
    char *input_copy = strdup(argv[2]);
    if (!input_copy) {
        perror("strdup failed");
        return 1;
    }

    // 3. Count the number of tokens (spaces)
    int token_count = 0;
    char *temp_str = strdup(input_copy); // Temp copy just for counting
    char *token = strtok(temp_str, " ");
    while (token != NULL) {
        token_count++;
        token = strtok(NULL, " ");
    }
    free(temp_str); // Clean up temp copy

    // 4. Allocate memory for the new argv array
    // Size = 1 (program name) + token_count + 1 (NULL terminator)
    char **new_argv = malloc((token_count + 2) * sizeof(char *));
    if (!new_argv) {
        perror("malloc failed");
        return 1;
    }

    // 5. Build the new argv list
    int new_argc = 0;
    
    // Keep the program name (argv[0])
    new_argv[new_argc++] = argv[0];

    // Tokenize the input_copy and add pointers to new_argv
    token = strtok(input_copy, " ");
    while (token != NULL) {
        new_argv[new_argc++] = token;
        token = strtok(NULL, " ");
    }
    
    // Null terminate the array (standard convention for argv)
    new_argv[new_argc] = NULL;

    // 6. Update main's variables to point to the new data
    // Note: The original argv[1] is now effectively removed/ignored
    argc = new_argc;
    argv = new_argv;
    printf("argv:\n");
    for(int i=0;i<argc;i++){
        printf("%s\n",argv[i]);
    }
    char *cmd_name = argv[1];
    printf("CMD NAME : %s\n",cmd_name);
    char *user = getenv("USER");
    if (!user) user = "unknown";

    // --- STEP 1: Identify Command Type ---
    CommandMetadata meta;
    
    // Check Internal Table first
    int found_internal = 0;
    for (int i = 0; internal_cmds[i].name != NULL; i++) {
        if (strcmp(cmd_name, internal_cmds[i].name) == 0) {
            meta = internal_cmds[i];
            found_internal = 1;
            break;
        }
    }

    // If not internal, treat as External Linux Command
    if (!found_internal) {
        meta.name = cmd_name;
        meta.is_internal = 0;
        meta.internal_func = NULL;
        meta.level = LVL_USER; // Default to safe
        meta.warning_msg = NULL;

        // Check against Dangerous Blacklist
        for (int i = 0; dangerous_binaries[i] != NULL; i++) {
            if (strcmp(cmd_name, dangerous_binaries[i]) == 0) {
                meta.level = LVL_DANGEROUS;
                meta.warning_msg = "This is a destructive system utility.";
                break;
            }
        }
    }

    // --- STEP 2: Policy Enforcement (PDP) ---
    
    // A. Check Permissions
    if (meta.level >= LVL_ADMIN) {
        if (!is_authorized_user()) {
            printf("PERMISSION DENIED: '%s' requires Admin privileges.\n", cmd_name);
            log_audit(cmd_name, user, "DENIED", "Unauthorized Group");
            return 1;
        }
    }

    // B. Check Safety (Warning System)
    if (meta.level == LVL_DANGEROUS) {
        printf("\n[KERNEL PROTECTION BOUNDARY]\n");
        printf("!!! WARNING: You are attempting to run '%s' !!!\n", cmd_name);
        printf("Reason: %s\n", meta.warning_msg);
        printf("Confirm execution? (yes/no): ");
        
        char resp[10];
        if (!fgets(resp, sizeof(resp), stdin) || strncmp(resp, "yes", 3) != 0) {
            printf("Operation Aborted.\n");
            log_audit(cmd_name, user, "ABORTED", "User declined warning");
            return 0;
        }
    }

    // --- STEP 3: Execution ---
    
    log_audit(cmd_name, user, "ALLOWED", "Passing to execution engine");

    if (meta.is_internal) {
        // Run internal C function
        meta.internal_func(argc, argv);
    } else {
        // Run external Linux command using execvp
        // We must shift argv so that argv[1] becomes the new argv[0]
        // Current argv: { "./kernel_dispatcher", "ls", "-la", NULL }
        // Needed argv:  { "ls", "-la", NULL }
        
        pid_t pid = fork();
        if (pid == 0) {
            // Child Process
            execvp(cmd_name, &argv[1]);
            
            // If execvp returns, it failed
            perror("[KERNEL] Execution failed");
            exit(1);
        } else if (pid > 0) {
            // Parent waits for child
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("Fork failed");
        }
        if(pid>0){
            printf("External Command executed successfully\n");
        }
    }

    return 0;
}
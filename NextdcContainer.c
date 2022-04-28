/*
	An application to run a process in container.
	Used for doing the research of Docker
	
	How to use:
			./NexdcContainer image-data/images image-data/container1
	
	Author: Zutao(Tony) Wu
	Email: vvilp.wu@gmail.com
*/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

// Print error message
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE);} while (0)

// defile stack
#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];

// sync primitive
int checkpoint[2];

// container root folder path
char* rootfs_path;

// container name
char* container_name;

// run a shell in the container for testing
char* const child_args[] = {
  "/usr/bin/sh",
  NULL
};

/*
	Run a shell command
	parameter 
		Linux command
	return 
		result of command
*/
char* exec(const char* command) {
  FILE* fp;
  char* line = NULL;
	// Following initialization is equivalent to char* result = ""; and just
	// initializes result to an empty string, only it works with
	// -Werror=write-strings and is so much less clear.
	char* result = (char*) calloc(1, 1);
	size_t len = 0;

	fflush(NULL);
	fp = popen(command, "r");
	if (fp == NULL) {
		printf("Cannot execute command:\n%s\n", command);
		return NULL;
	}

	while(getline(&line, &len, fp) != -1) {
		// +1 below to allow room for null terminator.
		result = (char*) realloc(result, strlen(result) + strlen(line) + 1);
		// +1 below so we copy the final null terminator.
		strncpy(result + strlen(result), line, strlen(line) + 1);
		free(line);
		line = NULL;
	}

	fflush(fp);
	if (pclose(fp) != 0) {
		perror("Cannot close stream.\n");
	}
	return result;
}

/*
	show the brief instruction of this command
*/
void show_command_intruction()
{
	printf("command: NextdcContainer run image container\n");
	printf(" -- image: should be exist file, can be created by make-devicemapper-files.sh\n");
	printf(" -- container: if container not exsit, will create\n");
}

/*
	create container folder with image data file and container data file.
	parameter 
		image data file path
		container data file path
*/
void cntner_device_mapper_init(char *image_path, char *container_path)
{
	char *cmd;
	if( access( container_path, F_OK ) != -1 ) {
		// file exists
	} else {
		//if container data file doesn't exist, create it 
		asprintf(&cmd, "dd if=/dev/zero of=%s bs=10M count=10", container_path);
		system(cmd);
		asprintf(&cmd, "mkfs.ext2 -F %s", container_path);
		system(cmd);
	}

	container_name = basename(container_path);
	
	char *rootfs;
	asprintf(&cmd, "mkdir %s_folder", container_path);
	asprintf(&rootfs, "%s_folder", container_path);
        rootfs_path = rootfs;
        system(cmd);

	asprintf(&cmd, "losetup --show --find %s", image_path);
        char *image_device = exec(cmd);
	
	asprintf(&cmd, "blockdev --getsz %s", image_path);
        char *image_device_size = exec(cmd);

	asprintf(&cmd, "losetup --show --find %s", container_path);
        char *container_device = exec(cmd);

        asprintf(&cmd, "dmsetup create '%s-mapper' --table '0 %s snapshot %s %s P 16'",container_name, image_device_size, image_device, container_device);
	system(cmd);	

	asprintf(&cmd, "mount /dev/mapper/%s-mapper %s",container_name, rootfs);
	system(cmd);
}

/*
	Setup cgroups by using libcgroup
*/
void cntner_create_cgroups()
{
	system("cgcreate -a nextdc -g memory,cpu:newgroup");
	//set 100mb memory limit
	system("echo 100000000 > /sys/fs/cgroup/memory/newgroup/memory.limit_in_bytes");
}

/*
	Set process into cgroups that we have created
*/
void cntner_set_pid_to_cgroups(int pid)
{
	char* cmd;
	asprintf(&cmd, "cgclassify -g cpu,memory:newgroup %d", pid);
	system(cmd);	
	printf("set cgroups finish !\n");
}

/*
	Setup network device in host
*/
void cntner_set_network_mainspace(int ns_id)
{
	system("ip link add A type veth peer name B");

	char* cmd;
	asprintf(&cmd, "ip link set B netns %d", ns_id);
	system(cmd);

	system("brctl addbr bridge0");
	system("ip addr add 172.17.42.1/16 dev bridge0");
	system("ip link set dev bridge0 up");
	system("brctl addif bridge0 A");
	system("ip link set A up ");

	system("iptables -F");
	system("iptables -t nat -A POSTROUTING -s 172.17.42.1/16  -d 0.0.0.0/0 -j MASQUERADE");
	printf("set_network_mainspace finish !\n");	
}

/*
	Setup network device in container
*/
void cntner_set_network_namespace()
{
	system("ip link set dev lo up");
	system("ip link set dev B name eth0");
	system("ip link set eth0 up");
	system("ip addr add 172.17.0.2/16 dev eth0");
	system("ip route add default via 172.17.42.1");
	printf("set_network_namespace finish !\n");	
}

/*
	Mount container to the folder which is created by device mapper function
	void cntner_device_mapper_init(char *image_path, char *container_path)
*/
void cntner_mount_sys()
{
	system("mount --make-rprivate /");

	char *cmd;

	//system("mount --rbind /sys /home/nextdc/arch-root/sys/");
	asprintf(&cmd, "mount --rbind /sys %s/sys/", rootfs_path);
	system(cmd);

	//system("mount --rbind /dev /home/nextdc/arch-root/dev/");
	asprintf(&cmd, "mount --rbind /dev %s/dev/", rootfs_path);
	system(cmd);

	//system("mount --rbind /run /home/nextdc/arch-root/run/");
	asprintf(&cmd, "mount --rbind /run %s/run/", rootfs_path);
	system(cmd);

	//system("mount --rbind /home/nextdc/arch-root /home/nextdc/arch-root");
	asprintf(&cmd, "mount --rbind %s %s", rootfs_path, rootfs_path);
	system(cmd);

	free(cmd);
	printf("mount system success !\n");
}

/*
	Copy necessary files to container
		/etc/resolv.conf
		/etc/hosts
		/etc/hostname
*/
void cntner_copy_sysfile_to_rootfs()
{
	char *cmd;

	//system("cp -rf /etc/resolv.conf /home/nextdc/arch-root/etc/resolv.conf");
	asprintf(&cmd, "cp -rf /etc/resolv.conf %s/etc/resolv.conf", rootfs_path);
	system(cmd);

	//system("cp -rf /etc/hosts /home/nextdc/arch-root/etc/hosts");
	asprintf(&cmd, "cp -rf /etc/hosts %s/etc/hosts", rootfs_path);
	system(cmd);

	//system("cp -rf /etc/hostname /home/nextdc/arch-root/etc/hostname");
	asprintf(&cmd, "cp -rf /etc/hostname %s/etc/hostname", rootfs_path);
	system(cmd);

	free(cmd);
}

/*
	Mount system folder inside the container
*/
void cntner_mount_sys_namespace()
{
	system("mount -t proc proc /proc");
}

void cntner_syslink()
{
	//system("ln -s /proc/self/fd /home/nextdc/arch-root/dev/fd");
	//system("ln -s /proc/self/fd/0 /home/nextdc/arch-root/dev/stdin");
	//system("ln -s /proc/self/fd/1 /home/nextdc/arch-root/dev/stdout");
	//system("ln -s /proc/self/fd/2 /home/nextdc/arch-root/dev/stderr");
	printf("set syslink finish !\n");
}

/*
	Using pivot_root to change the whole root system for particular process.
*/
void cntner_pivot_root_to_sys()
{
	char *cmd;
	char *old_root_name = "old-root";
	char *old_root_path;
	asprintf(&old_root_path, "%s/%s", rootfs_path, old_root_name);
	asprintf(&cmd, "rm -rf %s", old_root_path);
	system(cmd);
	asprintf(&cmd, "mkdir %s", old_root_path);
	system(cmd);
	pivot_root(rootfs_path, old_root_path);
}

/*
	Call Process inside container
*/
int child_main(void* arg)
{
	char c;

	close(checkpoint[1]);
	char **argv = arg;
	read(checkpoint[0], &c, 1);

	cntner_set_network_namespace();

	cntner_pivot_root_to_sys();

	cntner_mount_sys_namespace();

	execv(child_args[0], child_args);
	system("cd /");

	errExit("execvp");	    
	return 1;
}

/*
	Do some clear work after exit this application
*/
void cntner_clear(void)
{
	printf("------clear------\n");
	sleep(1);
	char *cmd;

	asprintf(&cmd, "umount -R -f -l %s/", rootfs_path);
	system(cmd);
	asprintf(&cmd, "umount -R -f -l %s/", rootfs_path);
	system(cmd);
	printf("%s\n",cmd);

	sleep(1);
	asprintf(&cmd, "dmsetup remove %s-mapper", container_name);
	system(cmd);
	printf("%s\n",cmd);

	sleep(1);
	asprintf(&cmd, "rmdir %s", rootfs_path);
	system(cmd);
	printf("%s\n",cmd);
	printf("------clear------\n");
}

int main(int argc, char *argv[])
{
	if(argc != 3) {
		show_command_intruction();
		return;
	}
	cntner_device_mapper_init(argv[1], argv[2]);
	pipe(checkpoint);
	cntner_mount_sys();
	cntner_copy_sysfile_to_rootfs();
	cntner_syslink();
	cntner_create_cgroups();
	cntner_set_pid_to_cgroups(getpid());
	int child_pid = clone(child_main, child_stack+STACK_SIZE,
	    CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | SIGCHLD, NULL);
	cntner_set_network_mainspace(child_pid);
	close(checkpoint[1]);
	waitpid(child_pid, NULL, 0);
	atexit (cntner_clear);
	exit (EXIT_SUCCESS);
	return 0;
}
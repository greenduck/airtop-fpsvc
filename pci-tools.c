/*
 * Copyright (C) 2016, CompuLab ltd.
 * Author: Andrey Gelman <andrey.gelman@compulab.co.il>
 * License: GNU GPLv2 or later, at your option
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#include "common.h"


#define SYS_PCI_DEVICES			"/sys/bus/pci/devices"
#define DRIVER_BUF_SIZE			128
#define PCI_CLASS_VGA			0x030000


typedef struct PciDevice {
	struct PciDevice *next;
	unsigned int pci_class;
	char driver[DRIVER_BUF_SIZE];
} PciDevice;


/*
 * Implement visitor pattern on each directory under 'path'.
 */
static void scan_dirs(const char *path, int (*visitor)(DIR *dir, void *arg), void *varg)
{
	DIR *root;
	DIR *dir;
	struct dirent *d;
	char buffer[128];
	bool keep_searching;

	root = opendir(path);
	if (root == NULL) {
		sloge("%s: could not open directory: %m", path);
		return;
	}

	keep_searching = true;
	while (((d = readdir(root)) != NULL) && keep_searching) {
		if (!strcmp(".", d->d_name) || !strcmp("..", d->d_name))
			continue;

		snprintf(buffer, sizeof(buffer), "%s/%s", path, d->d_name);
		dir = opendir(buffer);
		if (dir == NULL)
			continue;

		keep_searching = visitor(dir, varg);
		closedir(dir);
	}

	closedir(root);
}

static PciDevice *new_pci_device(PciDevice *dev, const char *drv_name)
{
	PciDevice *new_dev;

	new_dev = (PciDevice *)malloc(sizeof(PciDevice));
	new_dev->next = dev;
	if (dev != NULL)
		new_dev->pci_class = dev->pci_class;

	strcpy(new_dev->driver, drv_name);
	return new_dev;
}

/*
 * Return:
 * 'keep_searching' flag
 */
static int process_dir(DIR *dir, void *arg)
{
	PciDevice **ptr_dev = (PciDevice **)arg;
	PciDevice *dev = *ptr_dev;
	struct dirent *d;
	struct dirent *d_class = NULL;
	struct dirent *d_driver = NULL;
	char buffer[128];
	int fd;
	int n;

	while ((d = readdir(dir)) != NULL) {
		if (!strcmp("class", d->d_name))
			d_class = d;
		else if (!strcmp("driver", d->d_name))
			d_driver = d;
	}

	/* CLASS */
	if (d_class == NULL)
		return 1;

	fd = openat(dirfd(dir), d_class->d_name, O_RDONLY);
	if (fd < 0)
		return 1;

	read(fd, buffer, sizeof(buffer));
	close(fd);

	if (strtol(buffer, NULL, 0) != dev->pci_class)
		return 1;

	/* DRIVER */
	if (d_driver == NULL)
		return 1;

	n = readlinkat(dirfd(dir), d_driver->d_name, buffer, sizeof(buffer));
	if (n < 0)
		return 1;
	buffer[n] = '\0';

	*ptr_dev = new_pci_device(dev, basename(buffer));
	return 1;
}


static char *name_list = NULL;

char *vga_driver_name_list(void)
{
	PciDevice *vga_dev;
	PciDevice *p;
	int size;
	int size_left;

	if (name_list != NULL)
		return name_list;

	vga_dev = new_pci_device(NULL, "none");
	vga_dev->pci_class = PCI_CLASS_VGA;

	scan_dirs(SYS_PCI_DEVICES, process_dir, &vga_dev);


	/* convert vga_dev linked list into space-separated list of VGA driver names */
	size = 16;
	size_left = size;
	name_list = (char *)calloc(size, 1);
	while (vga_dev != NULL) {
		if (strlen(vga_dev->driver) > (size_left + 2)) {
			size += 16;
			size_left += 16;
			name_list = realloc(name_list, size);
			continue;
		}

		size_left -= sprintf((name_list + strlen(name_list)), "%s ", vga_dev->driver);
		p = vga_dev;
		vga_dev = p->next;
		free(p);
	}

	return name_list;
}


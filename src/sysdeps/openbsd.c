// SPDX-License-Identifier: GPL-3.0-or-later WITH GPL-3.0-linking-source-exception

/* GKrellM
|  Copyright (C) 1999-2019 Bill Wilson
|  
|  Author:  Bill Wilson    billw@gkrellm.net
|
|  OpenBSD code derived from FreeBSD code by: Hajimu UMEMOTO <ume@FreeBSD.org>
|  bsd-net-open.c code is Copyright (C):
|         Anthony Mallet <anthony.mallet@useless-ficus.net>
|
|
|  Latest versions might be found at:  http://gkrellm.net
*/

#include <kvm.h>

#include "../gkrellm.h"
#include "../gkrellm-private.h"
#include "../gkrellm-sysdeps.h"

kvm_t	*kvmd = NULL;
char	errbuf[_POSIX2_LINE_MAX];


void
gkrellm_sys_main_init(void)
	{
	/* We just ignore error, here.  Even if GKrellM doesn't have
	|  kmem privilege, it runs with available information.
	*/
	kvmd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf);
	if (setgid(getegid()) != 0)
		{
		fprintf(stderr, "Can't drop setgid privileges.");
		exit(1);
		}
	}

void
gkrellm_sys_main_cleanup(void)
	{
	}


/* ===================================================================== */
/* CPU monitor interface */

#include <sys/sysctl.h>
#include <sys/sched.h>

static gint ncpus;

static gint get_ncpus(void);

void
gkrellm_sys_cpu_read_data(void)
	{
	int64_t	cp_time[ncpus][CPUSTATES];
	size_t size;
	int i;

	size = sizeof(cp_time[0]);
	if (ncpus > 1) {
		for (i = 0; i < ncpus; i++) {
			int cp_time_mib[] = {CTL_KERN, KERN_CPTIME2, i};

			if (sysctl(cp_time_mib, 3, cp_time[i], &size, NULL, 0)
			    < 0)
				continue;

			gkrellm_cpu_assign_data(i, cp_time[i][CP_USER],
			    cp_time[i][CP_NICE], cp_time[i][CP_SYS],
			    cp_time[i][CP_IDLE]);
		}
	} else {
		int cp_time_mib[] = {CTL_KERN, KERN_CPTIME};
		long cp_time_tmp[CPUSTATES];

		if (sysctl(cp_time_mib, 2, cp_time_tmp, &size, NULL, 0) < 0)
			return;
		for (i = 0; i < CPUSTATES; i++)
			cp_time[0][i] = cp_time_tmp[i];

		gkrellm_cpu_assign_data(0, cp_time[0][CP_USER],
		    cp_time[0][CP_NICE], cp_time[0][CP_SYS],
		    cp_time[0][CP_IDLE]);

	}
}

gboolean
gkrellm_sys_cpu_init(void)
{
	ncpus = get_ncpus();
	gkrellm_cpu_set_number_of_cpus(ncpus);
	return TRUE;
}

static gint
get_ncpus(void)
{
	static int mib[] = { CTL_HW, HW_NCPU };
	int ncpus;
	size_t size = sizeof(int);

	if (sysctl(mib, 2, &ncpus, &size, NULL, 0) < 0)
		return 1;
	else
		return ncpus;
}

/* ===================================================================== */
/* Proc monitor interface */

#include <sys/sysctl.h>
#include <sys/vmmeter.h>

#include <utmp.h>

void
gkrellm_sys_proc_read_data(void)
{
   static int proc_mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
   static int fork_mib[] = { CTL_KERN, KERN_FORKSTAT };
   double avenrun;
	guint	n_forks = 0, n_processes = 0;
   size_t len;
   struct forkstat forkstat;

   if (sysctl(proc_mib, 3, NULL, &len, NULL, 0) >= 0) {
      n_processes = len / sizeof(struct kinfo_proc);
   }

   len = sizeof(forkstat);
   if (sysctl(fork_mib, 2, &forkstat, &len, NULL, 0) >= 0) {
      n_forks = forkstat.cntfork + forkstat.cntvfork;
   }

   if (getloadavg(&avenrun, 1) <= 0)
		avenrun = 0;
	gkrellm_proc_assign_data(n_processes, 0, n_forks, avenrun);
}

void
gkrellm_sys_proc_read_users(void)
	{
	gint	n_users;
   static time_t utmp_mtime;
   struct utmp utmp;
   struct stat s;
   FILE *ut;

	if (stat(_PATH_UTMP, &s) == 0 && s.st_mtime != utmp_mtime)
		{
		if ((ut = fopen(_PATH_UTMP, "r")) != NULL)
			{
			n_users = 0;
			while (fread(&utmp, sizeof(utmp), 1, ut))
				{
				if (utmp.ut_name[0] == '\0') continue;
					++n_users;
				}
			(void)fclose(ut);
			gkrellm_proc_assign_users(n_users);
			}
		utmp_mtime = s.st_mtime;
		}
	}

gboolean
gkrellm_sys_proc_init(void)
	{
	return TRUE;
	}


/* ===================================================================== */
/* Memory/Swap monitor interface */

#include <sys/vmmeter.h>
#include <sys/sysctl.h>
#include <uvm/uvm_extern.h>

static gulong	swapin,
				swapout;
static guint64	swap_total,
				swap_used;

void
gkrellm_sys_mem_read_data(void)
{
   static int vmtotal_mib[] = { CTL_VM, VM_METER };
   static int uvmexp_mib[] = { CTL_VM, VM_UVMEXP };
   static int pgout, pgin;
   unsigned long	total, used, x_used, free, shared, buffers, cached;
   struct vmtotal vmt;
   struct uvmexp uvmexp;
   size_t len;

   len = sizeof(vmt);
   if (sysctl(vmtotal_mib, 2, &vmt, &len, NULL, 0) < 0)
      memset(&vmt, 0, sizeof(vmt));

   len = sizeof(uvmexp);
   if (sysctl(uvmexp_mib, 2, &uvmexp, &len, NULL, 0) < 0)
      memset(&uvmexp, 0, sizeof(uvmexp));

   total = (uvmexp.npages - uvmexp.wired) << uvmexp.pageshift;

   /* not sure of what must be computed */
   x_used = (uvmexp.active + uvmexp.inactive) << uvmexp.pageshift;
   free = uvmexp.free << uvmexp.pageshift;
   shared = vmt.t_rmshr << uvmexp.pageshift;

   /* want to see only this in the chat. this could be changed */
   used = uvmexp.active << uvmexp.pageshift;

   /* don't know how to get those values */
   buffers = 0;
   cached = 0;

   gkrellm_mem_assign_data(total, used, free, shared, buffers, cached);

   /* show only the pages located on the disk and not in memory */
   swap_total = (guint64)uvmexp.swpages << uvmexp.pageshift;
   swap_used = (guint64)uvmexp.swpgonly << uvmexp.pageshift;

   /* For page in/out operations, uvmexp struct doesn't seem to be reliable */

   /* if the number of swapped pages that are in memory (inuse - only) is
    * greater that the previous value (pgin), we count this a "page in" */
   if (uvmexp.swpginuse - uvmexp.swpgonly > pgin)
      swapin += uvmexp.swpginuse - uvmexp.swpgonly - pgin;
   pgin = uvmexp.swpginuse - uvmexp.swpgonly;

   /* same for page out */
   if (uvmexp.swpgonly > pgout)
      swapout += uvmexp.swpgonly - pgout;
   pgout = uvmexp.swpgonly;
}

void
gkrellm_sys_swap_read_data(void)
	{
	gkrellm_swap_assign_data(swap_total, swap_used, swapin, swapout);
	}

gboolean
gkrellm_sys_mem_init(void)
	{
	return TRUE;
	}


/* ===================================================================== */
/* Sensor monitor interface */

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/sensors.h>
#include <errno.h>

static gboolean
get_sensor(int dev, int type, int num, gfloat *val)
{
	int mib[5] = { CTL_HW, HW_SENSORS };
	struct sensor sen;
	size_t len = sizeof(sen);

	mib[2] = dev;
	mib[3] = type;
	mib[4] = num;
	if (sysctl(mib, 5, &sen, &len, NULL, 0) == -1 ||
	    (SENSOR_FINVALID|SENSOR_FUNKNOWN) & sen.flags)
		return FALSE;

	*val = (gfloat)sen.value;
	return TRUE;
}

gboolean
gkrellm_sys_sensors_get_temperature(gchar *device_name, gint id,
		gint iodev, gint interface, gfloat *temp)
{
	return get_sensor(id, iodev, interface, temp);
}

gboolean
gkrellm_sys_sensors_get_fan(gchar *device_name, gint id,
		gint iodev, gint interface, gfloat *fan)
{
	return get_sensor(id, iodev, interface, fan);
}

gboolean
gkrellm_sys_sensors_get_voltage(gchar *device_name, gint id,
		gint iodev, gint interface, gfloat *volt)
{
	return get_sensor(id, iodev, interface, volt);
}

static gboolean
add_sensdev(int dev, struct sensordev *sensdev)
{
	static enum sensor_type stypes[] =
		{ SENSOR_TEMP, SENSOR_FANRPM, SENSOR_VOLTS_DC };
	static gint gtypes[] =
		{ SENSOR_TEMPERATURE, SENSOR_FAN, SENSOR_VOLTAGE };
	static gfloat fac[] = { 0.000001, 1.0, 0.000001 };
	static gfloat off[] = { -273.15, 0.0, 0.0 };
	char name[32];
	int mib[5] = { CTL_HW, HW_SENSORS };
	struct sensor sen;
	size_t len = sizeof(sen);
	int idx, num;
	gboolean found = FALSE;

	mib[2] = dev;
	for (idx = 0; sizeof(stypes) / sizeof(stypes[0]) > idx; idx++) {
		mib[3] = stypes[idx];
		for (num = 0; sensdev->maxnumt[stypes[idx]] > num; num++) {
			mib[4] = num;
			len = sizeof(sen);
			if (sysctl(mib, 5, &sen, &len, NULL, 0) == -1) {
				if (ENOENT != errno)
					return FALSE;
				continue;
			}
			if (SENSOR_FINVALID & sen.flags)
				continue;
			snprintf(name, sizeof(name), "%s.%s%d", sensdev->xname,
			    sensor_type_s[stypes[idx]], num);
			gkrellm_sensors_add_sensor(gtypes[idx], NULL, name,
			    sensdev->num, stypes[idx], num, fac[idx],
			    off[idx], NULL, (sen.desc[0] ? sen.desc : NULL));
			found = TRUE;
		}
	}

	return found;
}

gboolean
gkrellm_sys_sensors_init(void)
{
	int mib[3] = { CTL_HW, HW_SENSORS };
	struct sensordev sensdev;
	size_t len = sizeof(sensdev);
	int dev;
	gboolean found = FALSE;

#define GKRELLM_MAXSENSORDEVICES 1024
	for (dev = 0; GKRELLM_MAXSENSORDEVICES > dev; dev++) {
		mib[2] = dev;
		if (sysctl(mib, 3, &sensdev, &len, NULL, 0) == -1) {
			if (errno == ENXIO)
				continue;
			if (errno == ENOENT)
				break;
			return FALSE;
		}
		if (add_sensdev(dev, &sensdev))
			found = TRUE;
	}

	return found;
}


/* ===================================================================== */
/* Battery monitor interface */
#include <sys/ioctl.h>

#if defined(__i386__) || defined(__macppc__) || defined(__amd64__) || defined(__arm__) || defined(__sparc__) || defined(__sparc64__)

#include <machine/apmvar.h>
#define	APMDEV		"/dev/apm"

void
gkrellm_sys_battery_read_data(void)
	{
	int			f, r;
	struct apm_power_info info;
	gboolean    available, on_line, charging;
	gint        percent, time_left;

	if ((f = open(APMDEV, O_RDONLY)) == -1) return;
	memset(&info, 0, sizeof(info));
	r = ioctl(f, APM_IOC_GETPOWER, &info);
	close(f);
	if (r == -1) return;

	available = (info.battery_state != APM_BATT_UNKNOWN);
	on_line = (info.ac_state == APM_AC_ON) ? TRUE : FALSE;
	charging = (info.battery_state == APM_BATT_CHARGING) ? TRUE : FALSE;
	percent = info.battery_life;
	time_left = info.minutes_left;
	gkrellm_battery_assign_data(0, available, on_line, charging,
				percent, time_left);
	}

gboolean
gkrellm_sys_battery_init()
	{
	return TRUE;
	}

#else

void
gkrellm_sys_battery_read_data(void)
	{
	}

gboolean
gkrellm_sys_battery_init()
	{
	return FALSE;
	}
#endif


/* ===================================================================== */
/* Disk monitor interface */

#include <sys/sysctl.h>
#include <sys/disk.h>
#include <errno.h>

gchar *
gkrellm_sys_disk_name_from_device(gint device_number, gint unit_number,
			gint *order)
	{
	return NULL;	/* disk data by device not implemented */
	}

gint
gkrellm_sys_disk_order_from_name(const gchar *name)
	{
	return -1;  /* append disk charts as added */
	}


static int
get_diskinfo(struct diskstats **ret)
{
	static int mib[] = { CTL_HW, HW_DISKSTATS };
	static void *buf = NULL;
	static size_t buflen = 0;
	size_t len;
	void *newbuf;

	for (;;) {
		len = buflen;
		if (NULL != buf) {
			if (sysctl(mib, 2, buf, &len, NULL, 0) >= 0) {
				*ret = buf;
				return len / sizeof(struct diskstats);
			}
			if (ENOMEM != errno)
				return (-1);
		}

		if (sysctl(mib, 2, NULL, &len, NULL, 0) < 0)
			return (-1);
		len += sizeof(struct diskstats);
		if (NULL == (newbuf = realloc(buf, len)))
			return (-1);
		buf = newbuf;
		buflen = len;
	}
}

void
gkrellm_sys_disk_read_data(void)
{
	int diskcount, ii;
	struct diskstats *disks;

	diskcount = get_diskinfo(&disks);
	for (ii = 0; diskcount > ii; ii++)
		gkrellm_disk_assign_data_by_name(disks[ii].ds_name,
		    disks[ii].ds_rbytes, disks[ii].ds_wbytes, FALSE);
}

gboolean
gkrellm_sys_disk_init(void)
{
	return TRUE;
}

/* ===================================================================== */
/* Inet monitor interface */

#include "inet.h"

/* NO IPv6 SUPPORT YET */
#include <net/route.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

#include <kvm.h>

extern kvm_t *kvmd;

void gkrellm_sys_inet_read_tcp_data(void)
{
	ActiveTCP tcp;
	gint tcp_status;
	struct inpcbtable table;
	struct inpcb inpcb, *next;
	struct tcpcb tcpcb;
	static struct nlist nl[] = {
#define X_TCBTABLE 0
		{"_tcbtable"},
		{NULL}};

	if (kvmd == NULL)
		return;
	if (nl[0].n_type == 0)
		if (kvm_nlist(kvmd, nl) < 0 || nl[0].n_type == 0)
			return;

	if (kvm_read(kvmd, nl[X_TCBTABLE].n_value, (char *)&table,
				 sizeof(struct inpcbtable)) != sizeof(struct inpcbtable))
		return;

	next = TAILQ_FIRST(&table.inpt_queue);
	while (next != NULL)
	{
		if (kvm_read(kvmd, (u_long)next,
					 (char *)&inpcb, sizeof(inpcb)) == sizeof(inpcb) &&
			kvm_read(kvmd, (u_long)inpcb.inp_ppcb,
					 (char *)&tcpcb, sizeof(tcpcb)) == sizeof(tcpcb))
		{

			tcp.local_port = ntohs(inpcb.inp_lport);
			tcp.remote_addr.s_addr = inpcb.inp_faddr.s_addr;
			tcp.remote_port = ntohs(inpcb.inp_fport);
			tcp_status = (tcpcb.t_state == TCPS_ESTABLISHED);
			tcp.family = AF_INET;
			if (tcp_status == TCP_ALIVE)
				gkrellm_inet_log_tcp_port_data(&tcp);
		}

		next = TAILQ_NEXT(&inpcb, inp_queue);
	}
}

gboolean
gkrellm_sys_inet_init(void)
{
	return TRUE;
}

/*  Copyright 2007-2017 the original vcontrold development team

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <syslog.h>
#include <sys/param.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "common.h"
#include "semaphore.h"



int vcontrol_seminit()
{
    logIT(LOG_INFO, "vcontrol_seminit pid:%d", getpid());
    if (sem_unlink("/vcontrold") == -1) {
        logIT(LOG_ERR, "sem_unlink failed: %s", strerror(errno));
    }
    return 1;
}

int vcontrol_semfree()
{
    logIT(LOG_INFO, "Process %d tries to free semaphore", getpid());
    if (sem_unlink("/vcontrold") == -1) {
        logIT(LOG_ERR, "sem_unlink failed: %s", strerror(errno));
    }
    return 1; // what should we return?
}

int vcontrol_semget()
{
    int val = 0;
    logIT(LOG_INFO, "Process %d tries to aquire lock:", getpid());

    sem_t* sem = sem_open("/vcontrold", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        logIT(LOG_ERR, "sem_open failed: %s", strerror(errno));
        exit(1);
    }

    if (sem_getvalue(sem, &val) == -1) {
        logIT(LOG_ERR, "sem_getvalue failed: %s", strerror(errno));
    }

    if (val > 1) {
        logIT(LOG_ERR, "too many unlocks: %d", val);
    }

    if (sem_wait(sem) == -1) {
        logIT(LOG_ERR, "sem_wait failed: %s", strerror(errno));
        exit(1);
    }

    if (sem_close(sem) == -1) {
        logIT(LOG_ERR, "sem_close failed: %s", strerror(errno));
        exit(1);
    }

    logIT(LOG_INFO, "Process %d got lock %d", getpid(), val);
    return 1;
}

int vcontrol_semrelease()
{
    sem_t* sem = sem_open("/vcontrold", 0);
    if (sem == SEM_FAILED) {
        logIT(LOG_ERR, "sem_open failed: %s", strerror(errno));
        exit(1);
    }

    int val=0;
    sem_getvalue(sem, &val);

    if (sem_post(sem) == -1) {
        logIT(LOG_ERR, "sem_post failed: %s", strerror(errno));
        exit(1);
    }

    logIT(LOG_INFO, "Process %d released lock: %d", getpid(), val);

    if (sem_close(sem) == -1) {
        logIT(LOG_ERR, "sem_close failed: %s", strerror(errno));
        exit(1);
    }

    return 1;
}

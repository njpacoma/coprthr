/* cmdcall_e32pth_needham.c 
 *
 * Copyright (c) 2009-2012 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* DAR */

#undef _FORTIFY_SOURCE

#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CL/cl.h>

#include "xcl_structs.h"
#include "printcl.h"
#include "cmdcall.h"
#include "workp.h"
#include "e32pth_engine_needham.h"
#include "device.h"


static void* 
exec_ndrange_kernel(cl_device_id devid, void* p)
{
	printcl( CL_DEBUG "exec_ndrange_kernel");

	int i;

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;


	printcl( CL_DEBUG "argp->flags %x\n",argp->flags);
	printcl( CL_DEBUG "argp->k.krn %p\n",argp->k.krn);
	printcl( CL_DEBUG "argp->k.krn->narg %d\n",argp->k.krn->narg);
	printcl( CL_DEBUG "argp->k.krn->narg %d\n",argp->k.krn->narg);
	
	printcl( CL_DEBUG "argp->k.word_dim %d\n",argp->k.work_dim);
	printcl( CL_DEBUG "argp->k.global_work_offset[] %d %d %d\n",
		argp->k.global_work_offset[0],
		argp->k.global_work_offset[1],
		argp->k.global_work_offset[2]);
	printcl( CL_DEBUG "argp->k.global_work_size[] %d %d %d\n",
		argp->k.global_work_size[0],
		argp->k.global_work_size[1],
		argp->k.global_work_size[2]);
	printcl( CL_DEBUG "argp->k.local_work_size[] %d %d %d\n",
		argp->k.local_work_size[0],
		argp->k.local_work_size[1],
		argp->k.local_work_size[2]);

#define safe_div(a,b) ((b==0)? 0 : a/b)

	printcl( CL_DEBUG "argp->k.work_dim %d",argp->k.work_dim);

	struct workp_entry e0 = { 
		argp->k.work_dim,
		{ 
			argp->k.global_work_offset[0], 
			argp->k.global_work_offset[1], 
			argp->k.global_work_offset[2]
		},
		{
			argp->k.global_work_size[0],
			argp->k.global_work_size[1],
			argp->k.global_work_size[2]
		},
		{
			argp->k.local_work_size[0],
			argp->k.local_work_size[1],
			argp->k.local_work_size[2]
		},
		{ 0,0,0 },
		{
			safe_div(argp->k.global_work_size[0],argp->k.local_work_size[0]),
			safe_div(argp->k.global_work_size[1],argp->k.local_work_size[1]),
			safe_div(argp->k.global_work_size[2],argp->k.local_work_size[2])
		},
		{ 0,0,0 }

	};

	if (!e32pth_engine_ready_needham()) {
		int err = e32pth_engine_startup_needham((void*)devid);
		if (err != 0) return((void*)err);
	}
	
//	struct workp* wp = workp_alloc( 16 );
	struct workp* wp = workp_alloc( 1 );

	workp_init( wp );

	report_workp_entry(CL_DEBUG,&e0);

	workp_genpart( wp, &e0 );

	workp_reset(wp);
	struct workp_entry* e;

	while (e = workp_nxt_entry(wp)) 
		report_workp_entry(CL_DEBUG,e);

	int err = e32pth_engine_klaunch_needham(0,0,wp,argp);

	workp_free(wp);

	return((void*)err); 

}

static void* task(cl_device_id devid, void* argp)
{
	printcl( CL_WARNING "task: unsupported");
	return(0); 
}


static void* native_kernel(cl_device_id devid, void* argp) 
{
	printcl( CL_WARNING "native_kernel: unsupported");
	return(0); 
}


static void* read_buffer_safe(cl_device_id devid, void* p) 
{
	printcl( CL_DEBUG "read_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.src)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = argp->m.dst;
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst==src+offset) return(0);

	xxx_e_read_dram(src+offset,dst,len);

	return(0);
}

static void* read_buffer(cl_device_id devid, void* p) 
{
	printcl( CL_DEBUG "read_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.src)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = argp->m.dst;
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t offset = argp->m.src_offset;
	size_t len = argp->m.len;

	xxx_e_read_dram(src+offset,dst,len);

	return(0);
}


static void* write_buffer_safe(cl_device_id devid, void* p) 
{
	printcl( CL_DEBUG "write_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset;
	size_t len = argp->m.len;

	if (dst+offset == src) return(0);

	xxx_e_write_dram(dst+offset,src,len);

	return(0); 
}

static void* write_buffer(cl_device_id devid, void* p) 
{
	printcl( CL_DEBUG "write_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset;
	size_t len = argp->m.len;

	xxx_e_write_dram(dst+offset,src,len);

	return(0); 
}


static void* copy_buffer_safe(cl_device_id devid, void* p)
{
	printcl( CL_DEBUG "copy_buffer");

	/* XXX need to add this with a call on device for perofrmance -DAR */
	printcl( CL_ERR "not supported yet" );
	return(-1);

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t dst_offset = argp->m.dst_offset;
	size_t src_offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst+dst_offset == src+src_offset) return(0);

	else if (src+src_offset < dst+dst_offset+len 
		|| dst+dst_offset < src+src_offset+len) 
			memmove(dst+dst_offset,src+src_offset,len);
	
	else memcpy(dst+dst_offset,src+src_offset,len);

	return(0); 
}

static void* copy_buffer(cl_device_id devid, void* p)
{
	printcl( CL_DEBUG "copy_buffer");

#if(0)
	/* XXX need to add this with a call on device for perofrmance -DAR */
	printcl( CL_ERR "not supported yet" );
	return(-1);

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t dst_offset = argp->m.dst_offset;
	size_t src_offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst+dst_offset == src+src_offset) return(0);
	else memcpy(dst+dst_offset,src+src_offset,len);
#endif

	printcl( CL_WARNING 
		"copy_buffer using malloc+read_buffer+write_buffer+free");

	/* read_buffer */

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.src)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t offset = argp->m.src_offset;
	size_t len = argp->m.len;

	void* tmp = malloc(len);
	xxx_e_read_dram(src+offset,tmp,len);


	/* write_buffer */

	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];

	if (dst+offset == src) return(0);

	xxx_e_write_dram(dst+offset,tmp,len);
	if(tmp) free(tmp);

	/* free */

	return(0); 
}


static void* read_image(cl_device_id devid, void* p) 
{
	printcl( CL_DEBUG "read_image");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.src)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = argp->m.dst;
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t offset = argp->m.src_offset + 128;

	/* XXX here we should check for 3D image, ignore for now -DAR */

	size_t esz = 4 * sizeof(float);
	size_t w = *(size_t*)src;

	if (argp->m.region[0] == w) {

		size_t len = argp->m.region[1] * argp->m.region[0] * esz;
		memcpy(dst,src+offset,len);

	} else {

		size_t len = argp->m.region[0] * esz;
		for(n=0;n<argp->m.region[1];n++) memcpy(dst+n*w,src+offset+n*w,len);

	}
	

	return(0);
}


static void* write_image(cl_device_id devid, void* p) 
{
	printcl( CL_DEBUG "write_image");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset + 128;

	/* XXX here we should check for 3D image, ignore for now -DAR */

	size_t esz = 4 * sizeof(float);
	size_t w = *(size_t*)dst;

	if (argp->m.region[0] == w) {

		size_t len = argp->m.region[1] * argp->m.region[0] * esz;
		memcpy(dst+offset,src,len);

	} else {

		size_t len = argp->m.region[0] * esz;
		for(n=0;n<argp->m.region[1];n++) memcpy(dst+offset+n*w,src+n*w,len);

	}
	

	return(0); 
}


static void* copy_image(cl_device_id devid, void* argp) 
{
	printcl( CL_WARNING "copy_image: unsupported");
	return(0); 
}


static void* copy_image_to_buffer(cl_device_id devid, void* argp) 
{
	printcl( CL_WARNING "copy_image_to_buffer: unsupported");
	return(0); 
}


static void* copy_buffer_to_image(cl_device_id devid, void* argp)
{
	printcl( CL_WARNING "copy_buffer_to_image: unsupported");
	return(0); 
}


static void* map_buffer(cl_device_id devid, void* argp) 
{
	printcl( CL_WARNING "map_buffer: unsupported");
	return(0); 
}


static void* map_image(cl_device_id devid, void* argp) 
{
	printcl( CL_WARNING "map_image: unsupported");
	return(0); 
}


static void* unmap_mem_object(cl_device_id devid, void* argp) 
{
	printcl( CL_WARNING "unmap_mem_object: unsupported");
	return(0); 
}


static void* marker(cl_device_id devid, void* p) 
{
	printcl( CL_DEBUG "marker");
	return(0); 
}


static void* acquire_gl_objects(cl_device_id devid, void* argp)
{
	printcl( CL_WARNING "acquire_gl_objects: unsupported");
	return(0); 
}


static void* release_gl_objects(cl_device_id devid, void* argp) 
{
	printcl( CL_WARNING "acquire_gl_objects: unsupported");
	return(0); 
}



/* 
 * XXX The *_safe versions of read/write/copy_buffer should be used for
 * XXX careful treatment of memory region overlap.  This can be added as
 * XXX a runtime option that simply modifies the cmdcall table. -DAR
 */

cmdcall_t cmdcall_e32pth_needham[] = {
	0,
	exec_ndrange_kernel,
	task,
	native_kernel,
	read_buffer,	
	write_buffer,	
	copy_buffer,	
	read_image,
	write_image,
	copy_image,
	copy_image_to_buffer,
	copy_buffer_to_image,
	map_buffer,
	map_image,
	unmap_mem_object,
	marker,
	acquire_gl_objects,
	release_gl_objects
};



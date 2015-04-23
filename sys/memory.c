#include <sys/memory.h>
#include <sys/pmmgr.h>
#include <sys/sbunix.h>
#include <sys/process.h>
#include <sys/page.h>
#include <sys/utility.h>

p_fmgr pfmgr_head=NULL;//pfmgr_head points to the pfmgr_head of the memory linked list
p_fmgr pfmgr_tail=NULL;//pfmgr_tail points to the last node of the linked list, brk is data chunk + meta
void* page_phys_addr = NULL;
p_fmgr frame_manager_last = NULL;
p_fmgr frame_manager_start = NULL;


void* kmalloc(size_t size)
{

	if(size <= 0)
		return NULL;
	//allign the size in multiples of 8
	size = ALIGN(size);
	void* ret_addr = alloc_addr(size);
	if(ret_addr != NULL){
	  memset1((char*)ret_addr, 0, size);
	}
	return ret_addr;
	
}

void* alloc_addr(size_t size)
{
	p_fmgr temp = NULL, prev = NULL;
	void* ret_addr = NULL;

	if(!pfmgr_head)
	{
		//allocate a page for the frame_manager	to store the metadata of the malloc'ed pages
		pfmgr_head = new_page_mgr_alloc(PAGE_SIZE);
		if(!pfmgr_head)
			return NULL;
		frame_manager_start = pfmgr_head;
		printf("Pfmgr_Head is %p\n", pfmgr_head);

		//frame_manager_last tracks the last metadata
		frame_manager_last = pfmgr_head;

		//creates the actual page and adds the metadata to the pfmgr_head
		if(!init_page(pfmgr_head,size))
		{
			return NULL;
		}

		//allocate size to the page and update the metadata in the frame_manager
		if(!add_mgr_node(pfmgr_head, size))
		{
			return NULL;
		}

		ret_addr = (void*)((uint64_t)pfmgr_head->frame_start_addr + (uint64_t)pfmgr_head->offset);
		return ret_addr;
	}

	//traverse the frame_manager list and check if we can find a chunk with 'size' size.
	temp = pfmgr_head;
	while(temp != NULL)
	{
		
		if((temp->free == 1) && (temp->size == size)){
			temp->free=0;
		}
		else if((temp->free == 1) && (temp->size > size))
		{
			if(!add_mgr_node(temp, size))
			{
				return NULL;
			}
			ret_addr = (void*)((uint64_t)temp->frame_start_addr + (uint64_t)temp->offset);

			return ret_addr;
		}
		prev = temp;
		temp = temp->next;
	}

	//we couldn't allocate a chunk in the existing frame_manager list
	//so create a new page and add to the existing frame_manager lise
	
	p_fmgr node=NULL;
	/* check if there is space for one more pf_mgr in the current pf_mgr block */
	if((frame_manager_start + ENTRIES_PER_FRAME_MGR) == frame_manager_last){
		frame_manager_start = new_page_mgr_alloc(PAGE_SIZE);
		if(!frame_manager_start){
				return NULL;
		}
		frame_manager_last->next = frame_manager_start;		
		node = frame_manager_start;		
	}
	else{
		node = frame_manager_last + 1;
	}

	
	prev->next = node;
	frame_manager_last = node;

	if(!init_page(node,size))
	{
		return NULL;
	}
	if(!add_mgr_node(node, size))
	{
		return NULL;
	}
	ret_addr = (void*)((uint64_t)node->frame_start_addr + (uint64_t)node->offset);

	return ret_addr;
}

void* init_page(p_fmgr node,size_t size)
{
	page_phys_addr = (void*) get_virt_addr((uint64_t)alloc_frame(size));	
	if(!page_phys_addr)
	{
		return NULL;
	}
	memset1((char *)page_phys_addr,0,size);
	node->frame_start_addr = page_phys_addr;
	node->offset = 0;
	node->next = NULL;
	node->free = 1;
	node->size = PAGE_SIZE*(size%PAGE_SIZE==0 ? size/PAGE_SIZE : size/PAGE_SIZE+1);
	return (void*)node; 
}

void* add_mgr_node(p_fmgr node, size_t size)
{
	p_fmgr new_node;
	
	//check if frame_manager is full or not
	if((frame_manager_start + ENTRIES_PER_FRAME_MGR) == frame_manager_last)
	{
		frame_manager_start = new_page_mgr_alloc(PAGE_SIZE);
		if(!frame_manager_start)
		{
			return NULL;
		}
		frame_manager_last->next = frame_manager_start;
		new_node = frame_manager_start;
	}
	else
	{
		//create a new node at the end of the frame_manager list and update the frame_manager_last
		new_node = frame_manager_last + 1;	
	}
	
	frame_manager_last = new_node;

	new_node->frame_start_addr = node->frame_start_addr;
	new_node->offset = node->offset + size;
	new_node->next = node->next;
	new_node->size = node->size - size;
	new_node->free = 1;

	node->free = 0;
	node->size = size;
	node->next = new_node;
	return (void*)node;
}

void* new_page_mgr_alloc(size_t size)
{
	void* frame_manager_phys_addr = alloc_frame(size);
	memset1((char *)get_virt_addr((uint64_t)frame_manager_phys_addr),0,size);
	if(!frame_manager_phys_addr)
	{
		return NULL;
	}
	p_fmgr node = (p_fmgr)get_virt_addr((uint64_t) frame_manager_phys_addr);
	printf("new_page_mgr address is %p\n", node);

	return (void*)node;
}


void kfree(void* addr)
{
	p_fmgr temp;
	temp = pfmgr_head;

	while(temp != NULL)
	{
		if((void*)((uint64_t)temp->frame_start_addr + (uint64_t)temp->offset) == addr)
		{
			temp->free = 1;
			break;
		}
		temp = temp->next;
	}
}

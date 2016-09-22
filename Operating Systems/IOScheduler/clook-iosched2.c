/*
 * CLOOK Algorithm by Carlos Alva, JB, SG
 * COP 4610 Operating Systems
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct clook_data {
	struct list_head queue;
};

int sector_pos = -100;//Initialize to negative so it is alwasy less than 0 at worse

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (!list_empty(&nd->queue)) {
		struct request *rq;
		rq = list_entry(nd->queue.next, struct request, queuelist);
		
		//Saves last sector position so it can be used to sort request queue list		
		sector_pos = blk_rq_pos(rq);	

		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);

		char *dir;
		if(rq_data_dir(rq) == 0)
			dir = "READ";
		else dir = "WRITE";


		printk("[CLOOK] dsp %s %lu\n", dir, blk_rq_pos(rq));

		return 1;
	}
	return 0;
}

static void clook_add_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;
	struct list_head *curr = NULL;
	struct request *curr_request = NULL;

	//If nd->queue is empty, it would simple skip the loop
	list_for_each(curr, &nd->queue){
		curr_node = list_entry(curr, struct request, queuelist);
		//If the request sector is higher than the current sector position
		if(sector_pos =< blk_rq_pos(rq){
			//If current node sector is higher than request OR it is lower than current sector position
			//then break so request is added before curr_node			
			if (blk_rq_pos(curr_node) >= blk_rq_pos(rq)){
				break;
			}
			if (blk_rq_pos(curr_node) =< sector_pos){
				break;
			}

		}
		else {//If the request sector is lower than the current sector position
			
			//If current node is higher than request AND is lower than current sector position
			if (blk_rq_pos(curr_node) >= blk_rq_pos(rq)){				
				if(blk_rq_pos(curr_node) =< sector_pos){
					break;
				}
			}
			
		}

	}
	
	char *dir;
	if(rq_data_dir(rq) == 0)
		dir = "READ";
	else dir = "WRITE";


	printk("[CLOOK] add %s %lu\n", dir, blk_rq_pos(rq));

	//Adds rq->queuelist before curr node
	//If nd->queue empty then add before curr, if curr points to head of list then adds to the tail of the list
	list_add_tail(&rq->queuelist, curr);
}

static int clook_queue_empty(struct request_queue *q)
{
	struct clook_data *nd = q->elevator->elevator_data;

	return list_empty(&nd->queue);
}

static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
clook_latter_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static void *clook_init_queue(struct request_queue *q)
{
	struct clook_data *nd;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd)
		return NULL;
	INIT_LIST_HEAD(&nd->queue);
	return nd;
}

static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_queue_empty_fn	= clook_queue_empty,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_latter_req_fn		= clook_latter_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	elv_register(&elevator_clook);

	return 0;
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("Carlos Alva, JB, SG");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CLOOK I/O Scheduler");

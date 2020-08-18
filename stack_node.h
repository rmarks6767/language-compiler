/// @author: tvf

#ifndef STACK_NODE_H
#define STACK_NODE_H

/// Represents a single node in the stack
typedef struct stack_node_s {
	void *data;		        ///< data associated with the node	
	struct stack_node_s *next;	///< pointer to next node (NULL if none)
} stack_node_t;

#endif

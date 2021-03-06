/** @file checkpoint_handlers.h
 * @brief Handlers related to checkpoints.
 */
/* Copyright (c) 2011 Richard Boulton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef RESTPOSE_INCLUDED_CHECKPOINT_HANDLERS_H
#define RESTPOSE_INCLUDED_CHECKPOINT_HANDLERS_H

#include "rest/handler.h"

/** Create a checkpoint.
 *
 *  Expects 1 path parameter
 *
 *   - the collection name
 */
class CollCreateCheckpointHandlerFactory : public HandlerFactory {
  public:
    Handler * create(const std::vector<std::string> & path_params) const;
};
class CollCreateCheckpointHandler : public QueuedHandler {
    std::string coll_name;
  public:
    CollCreateCheckpointHandler(const std::string & coll_name_)
	    : coll_name(coll_name_)
    {}

    /** Create a new checkpoint, and add it to the appropriate queue.
     *
     * @param coll_name The name of the collection to index to.
     * @param checkid Will be set to the id of the newly created checkpoint.
     * @param do_commit If true, the checkpoint will cause a commit.
     * Otherwise, the checkpoint will just enforce ordering of operations.
     * @param allow_throttle If true, don't queue the document is the queue is
     * already busy.  Otherwise, queue the document unless the queue is
     * completely full.
     *
     * Allow throttle should be specified for documents being read from a
     * stream, if the source of documents can be paused to allow the queue to
     * catch up.  This allows other sources of documents for indexing (such as
     * pushes to the API) to have a chance at getting onto the queue.
     */
    static Queue::QueueState create_checkpoint(TaskManager * taskman,
					       const std::string & coll_name,
					       std::string & checkid,
					       bool do_commit,
					       bool allow_throttle);


    Queue::QueueState enqueue(ConnectionInfo & conn,
			      const Json::Value & body);
};


/** Get the list of all checkpoints in a collection.
 *
 *  Expects 1 path parameter:
 *
 *   - the collection name
 */
class CollGetCheckpointsHandlerFactory : public HandlerFactory {
  public:
    Handler * create(const std::vector<std::string> & path_params) const;
};
class CollGetCheckpointsHandler : public QueuedHandler {
    std::string coll_name;
  public:
    CollGetCheckpointsHandler(const std::string & coll_name_)
	    : coll_name(coll_name_)
    {}

    Queue::QueueState enqueue(ConnectionInfo & conn,
			      const Json::Value & body);
};


/** Get the status of a checkpoint.
 *
 *  Expects 2 path parameters:
 *
 *   - the collection name
 *   - the checkpoint id
 */
class CollGetCheckpointHandlerFactory : public HandlerFactory {
  public:
    Handler * create(const std::vector<std::string> & path_params) const;
};
class CollGetCheckpointHandler : public QueuedHandler {
    std::string coll_name;
    std::string checkid;
  public:
    CollGetCheckpointHandler(const std::string & coll_name_,
			     const std::string & checkid_)
	    : coll_name(coll_name_),
	      checkid(checkid_)
    {}

    Queue::QueueState enqueue(ConnectionInfo & conn,
			      const Json::Value & body);
};

#endif /* RESTPOSE_INCLUDED_CHECKPOINT_HANDLERS_H */

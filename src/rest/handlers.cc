/** @file handlers.cc
 * @brief Definition of concrete handlers
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

#include <config.h>
#include "rest/handlers.h"

#include "httpserver/httpserver.h"
#include "logger/logger.h"
#include <microhttpd.h>
#include "server/task_manager.h"
#include "server/tasks.h"
#include "utils/jsonutils.h"

using namespace std;
using namespace RestPose;

#define DIR_SEPARATOR "/"

Handler *
RootHandlerFactory::create(const vector<string> &) const
{
    return new FileHandler("static" DIR_SEPARATOR "index.html");
}

Handler *
FileHandlerFactory::create(const vector<string> &path_params) const
{
    string filepath("static" DIR_SEPARATOR "static");
    for (vector<string>::const_iterator i = path_params.begin();
	 i != path_params.end(); ++i) {
	filepath += DIR_SEPARATOR;
	filepath += *i;
    }

    return new FileHandler(filepath);
}

Queue::QueueState
FileHandler::enqueue(ConnectionInfo &,
		     const Json::Value &)
{
    return taskman->queue_readonly("static",
	new StaticFileTask(resulthandle, path));
}


Handler *
ServerStatusHandlerFactory::create(const std::vector<std::string> &) const
{
    return new ServerStatusHandler;
}

Queue::QueueState
ServerStatusHandler::enqueue(ConnectionInfo &,
			     const Json::Value &)
{
    return taskman->queue_readonly("status",
	new ServerStatusTask(resulthandle, taskman));
}

Handler *
CollCreateHandlerFactory::create(
	const std::vector<std::string> & path_params) const
{
    string coll_name = path_params[0];
    return new CollCreateHandler(coll_name);
}

Queue::QueueState
CollCreateHandler::enqueue(ConnectionInfo &,
			   const Json::Value &)
{
    return Queue::FULL; // FIXME
}

Handler *
IndexDocumentHandlerFactory::create(
	const std::vector<std::string> & path_params) const
{
    LOG_INFO("IndexDocumentHandler called");
    string coll_name = path_params[0];
    string doc_type = path_params[1];
    string doc_id = path_params[2];
    return new IndexDocumentHandler(coll_name, doc_type, doc_id);
}

Queue::QueueState
IndexDocumentHandler::enqueue(ConnectionInfo &,
			      const Json::Value & body)
{
    return taskman->queue_processing(coll_name,
	new ProcessorProcessDocumentTask(doc_type, doc_id, body),
	false);
}

Handler *
CollListHandlerFactory::create(const std::vector<std::string> &) const
{
    return new CollListHandler;
}

Queue::QueueState
CollListHandler::enqueue(ConnectionInfo &,
			 const Json::Value &)
{
    return taskman->queue_readonly("info",
	new CollListTask(resulthandle, taskman->get_collections()));
}

Handler *
CollInfoHandlerFactory::create(
	const std::vector<std::string> & path_params) const
{
    string coll_name = path_params[0];
    return new CollInfoHandler(coll_name);
}

Queue::QueueState
CollInfoHandler::enqueue(ConnectionInfo &,
			 const Json::Value &)
{
    return taskman->queue_readonly("info",
	new CollInfoTask(resulthandle, coll_name));
}

Handler *
SearchHandlerFactory::create(const std::vector<std::string> & path_params) const
{
    string coll_name = path_params[0];
    string doc_type = path_params[1];
    return new SearchHandler(coll_name, doc_type);
}

Queue::QueueState
SearchHandler::enqueue(ConnectionInfo &,
		       const Json::Value & body)
{
    return taskman->queue_readonly("search",
	new PerformSearchTask(resulthandle, coll_name, body, doc_type));
}

Handler *
GetDocumentHandlerFactory::create(const std::vector<std::string> & path_params) const
{
    string coll_name = path_params[0];
    string doc_type = path_params[1];
    string doc_id = path_params[2];
    return new GetDocumentHandler(coll_name, doc_type, doc_id);
}

Queue::QueueState
GetDocumentHandler::enqueue(ConnectionInfo &,
			    const Json::Value &)
{
    return taskman->queue_readonly("search",
	new GetDocumentTask(resulthandle, coll_name, doc_type, doc_id));
}


Handler *
NotFoundHandlerFactory::create(const std::vector<std::string> &) const
{
    return new NotFoundHandler;
}

void
NotFoundHandler::handle(ConnectionInfo & conn)
{
    conn.respond(MHD_HTTP_NOT_FOUND, "Resource not found", "text/plain");
}

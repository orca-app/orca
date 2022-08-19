/************************************************************//**
*
*	@file: osx_surface_client.cpp
*	@author: Martin Fouilleul
*	@date: 19/08/2022
*	@revision:
*
*****************************************************************/

#include"graphics_internal.h"

//------------------------------------------------------------------------------------------------
// private interfaces that need to be declared explicitly...
typedef uint32_t CGSConnectionID;
CGSConnectionID CGSMainConnectionID(void);

typedef uint32_t CAContextID;

@interface CAContext : NSObject
{
}
+ (id)contextWithCGSConnection:(CAContextID)contextId options:(NSDictionary*)optionsDict;
@property(readonly) CAContextID contextId;
@property(retain) CALayer *layer;
@end

@interface CALayerHost : CALayer
{
}
@property CAContextID contextId;
@end
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// Surface server
//------------------------------------------------------------------------------------------------

typedef struct mg_gles_surface_server
{
	mg_surface_server_info interface;
	CAContext* context;
} mg_gles_surface_server;

void mg_gles_surface_server_destroy(mg_surface_server_info* interface)
{
	mg_gles_surface_server* server = (mg_gles_surface_server*)interface;
	@autoreleasepool
	{
		[server->context release];
	}
}

mg_surface_server_id mg_gles_surface_server_get_id(mg_surface_server_info* interface)
{
	mg_gles_surface_server* server = (mg_gles_surface_server*)interface;

	@autoreleasepool
	{
		CAContextID contextID = [server->context contextId];
		return((void*)(uintptr_t)contextID);
	}
}

mg_surface_server mg_gles_surface_create_server(mg_surface_info* interface)
{@autoreleasepool{

	mg_gles_surface* surface = (mg_gles_surface*)interface;

	mg_gles_surface_server* server = malloc_type(mg_gles_surface_server);
	server->interface.destroy = mg_gles_surface_server_destroy;
	server->interface.getID = mg_gles_surface_server_get_id;

	NSDictionary* dict = [[NSDictionary alloc] init];
	CGSConnectionID connectionID = CGSMainConnectionID();
	server->context = [CAContext contextWithCGSConnection: connectionID options: dict];
	[server->context retain];
	[server->context setLayer: surface->layer];

	mg_surface_server handle = mg_surface_server_alloc_handle((mg_surface_server_info*)server);
	return(handle);
}}


mg_surface_server mg_gles_surface_server_create_native(void* p)
{@autoreleasepool{

	mg_gles_surface_server* server = malloc_type(mg_gles_surface_server);
	server->interface.destroy = mg_gles_surface_server_destroy;
	server->interface.getID = mg_gles_surface_server_get_id;

	NSDictionary* dict = [[NSDictionary alloc] init];
	CGSConnectionID connectionID = CGSMainConnectionID();
	server->context = [CAContext contextWithCGSConnection: connectionID options: dict];
	[server->context retain];
	[server->context setLayer: (CALayer*)p];

	mg_surface_server handle = mg_surface_server_alloc_handle((mg_surface_server_info*)server);
	return(handle);
}}

//------------------------------------------------------------------------------------------------
// Surface client
//------------------------------------------------------------------------------------------------
typedef struct mg_osx_surface_client
{
	mg_surface_client_info interface;
	CALayerHost* layerHost;

} mg_osx_surface_client;

void mg_osx_surface_client_destroy(mg_surface_client_info* interface)
{
	mg_osx_surface_client* client = (mg_osx_surface_client*)interface;
	@autoreleasepool
	{
		[client->layerHost release];
	}
}

void mg_osx_surface_client_attach(mg_surface_client_info* interface)
{
	mg_osx_surface_client* client = (mg_osx_surface_client*)interface;

	mp_view_data* viewData = mp_view_ptr_from_handle(interface->attachment);
	[viewData->nsView setWantsLayer:YES];

	CGRect bounds = [viewData->nsView bounds];
	CALayer* layer = [viewData->nsView layer];
	[layer addSublayer:client->layerHost];

	[client->layerHost setPosition:CGPointMake(bounds.size.width/2, bounds.size.height/2)];
}

void mg_osx_surface_client_detach(mg_surface_client_info* interface)
{
	mg_osx_surface_client* client = (mg_osx_surface_client*)interface;

	mp_view_data* viewData = mp_view_ptr_from_handle(interface->attachment);
	[client->layerHost removeFromSuperlayer];
}

mg_surface_client mg_osx_surface_client_create(mg_surface_server_id ID)
{
	mg_osx_surface_client* client = malloc_type(mg_osx_surface_client);

	client->interface.destroy = mg_osx_surface_client_destroy;
	client->interface.attach = mg_osx_surface_client_attach;
	client->interface.detach = mg_osx_surface_client_detach;

	@autoreleasepool
	{
		CAContextID contextID = (CAContextID)((uintptr_t)ID);

		client->layerHost = [[CALayerHost alloc] init];
		[client->layerHost retain];
		[client->layerHost setContextId: contextID];
	}
	mg_surface_client handle = mg_surface_client_alloc_handle((mg_surface_client_info*)client);
	return(handle);
}

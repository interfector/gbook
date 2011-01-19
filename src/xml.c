#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/parser.h>

#include <gbook.h>

extern struct gbook_engine gamebook;
extern struct user_configs user_cfg;
extern struct gbook_merchant merchant;

char*
replace( char* start, char* nid, char* rep )
{
	char* replaced;
	char* temp;
	char* found;

	found = strstr( start, nid );

	if( !found )
		return NULL;

	replaced = found;

	temp = strdup( found + strlen(nid) );

	replaced = malloc( (found - start) + strlen(rep) + strlen(temp) + 1);
	memset( replaced, 0, (found - start) + strlen(rep) + strlen(temp) + 1);

	memcpy( replaced, start, (found - start) );
	strcat( replaced, rep );
	strcat( replaced, temp );

	free( temp );

	return replaced;
}

char*
strreplace( char* start, char* nid, char* rep )
{
	char* srep = strdup( start );
	char* temp;

	while(strstr(srep, nid))
	{
		temp = replace( srep, nid, rep );

		free( srep );

		srep = strdup( temp );

		free( temp );
	}

	return srep;
}

int
_atoi(char* string)
{
	int sign, number;

	for(number = sign = 0;*string;string++)
		if(*string >= '0' && *string <= '9')
			number = number * 10 + ((int)*string - '0');
		else if( *string == '-' && number == 0)
			sign = 1;

	return (sign) ? -number : number;
}

void
loadProfile(const char * file)
{
	LIBXML_TEST_VERSION

	int i = 0, j;
	char* token, *string, *tmp;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;

	if ((doc = xmlReadFile (file, NULL, 0)) == NULL)
	{
		fprintf (stderr, "Error: couldn't parse file %s\n", file);
		exit(1);
	}

	node = xmlDocGetRootElement (doc);

	if (xmlStrcmp (node->name, (xmlChar *) "configs"))
	{
		fprintf (stderr, "Error when loading configuration\n");
		exit( 1 );
	}

	node = ((xmlNodePtr)node->children);

	while(node)
	{
		if (node->type == XML_ELEMENT_NODE && node->children->type == XML_TEXT_NODE)
		{
			if(!strcmp(xmlStrdup(node->name), "name"))
			{
				user_cfg.name = xmlStrdup(node->children->content);
			} else if(!strcmp(xmlStrdup(node->name), "money"))
			{
				user_cfg.money = _atoi(xmlStrdup(node->children->content));
			} else if(!strcmp(xmlStrdup(node->name), "life"))
			{
				user_cfg.lifep = _atoi(xmlStrdup(node->children->content));
			} else if(!strcmp(xmlStrdup(node->name), "object"))
			{
				if(!user_cfg.objects)
					user_cfg.objects = malloc( CH_BLOCK * sizeof(char*) );

				if( user_cfg.n_object % CH_BLOCK )
					user_cfg.objects = realloc(user_cfg.objects, (CH_BLOCK + user_cfg.n_object) * sizeof(char*) );

				user_cfg.objects[ user_cfg.n_object++ ] = xmlStrdup( node->children->content );
			}

		}
		node = node->next;
	}
}

void
loadMerchant(const char * file  )
{
	LIBXML_TEST_VERSION

	int i = 0, j;
	char* token, *string, *tmp;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;

	if ((doc = xmlReadFile (file, NULL, 0)) == NULL)
	{
		fprintf (stderr, "Error: couldn't parse file %s\n", file);
		exit(1);
	}

	node = xmlDocGetRootElement (doc);

	if (xmlStrcmp (node->name, (xmlChar *) "merchant"))
	{
		fprintf (stderr, "Error when loading configuration\n");
		exit( 1 );
	}

	node = ((xmlNodePtr)node->children);

	while(node)
	{
		if (node->type == XML_ELEMENT_NODE && node->children->type == XML_TEXT_NODE)
		{
			if(!strcmp(xmlStrdup(node->name), "object"))
			{
				if(!merchant.objs)
					merchant.objs = malloc( CH_BLOCK * sizeof(struct gbook_object) );

				if( merchant.n_obj % CH_BLOCK )
					merchant.objs = realloc(merchant.objs, (CH_BLOCK + merchant.n_obj) * sizeof(struct gbook_object) );

				merchant.objs[ merchant.n_obj ].name = xmlStrdup( node->children->content );
				merchant.objs[ merchant.n_obj++ ].money = _atoi(xmlStrdup( node->properties->children->content ));
			}

		}
		node = node->next;
	}
}

void
loadBook(const char * file)
{
	LIBXML_TEST_VERSION

	int i = 0, j;
	char* token, *string, *tmp;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;
	xmlNodePtr node_child = NULL;
	xmlAttrPtr prop = NULL;

	if ((doc = xmlReadFile (file, NULL, 0)) == NULL)
	{
		fprintf (stderr, "Error: couldn't parse file %s\n", file);
		exit(1);
	}

	node = xmlDocGetRootElement (doc);

	if (xmlStrcmp (node->name, (xmlChar *) "story"))
	{
		fprintf (stderr, "Error when loading configuration\n");
		exit(1);
	}

	node = ((xmlNodePtr)node->children);

	while (node)
	{
		if (node->type == XML_ELEMENT_NODE && node->children->type == XML_TEXT_NODE)
		{
			#ifdef _DEBUG
				printf("%s -> \n", xmlStrdup(node->name) );
			#endif

			if(!gamebook.action)
				gamebook.action = malloc( sizeof(struct gbook_act) * CH_BLOCK );

			if(gamebook.n_action % CH_BLOCK)
				gamebook.action = realloc( gamebook.action, (gamebook.n_action + CH_BLOCK) * sizeof(struct gbook_act) );

			gamebook.action[gamebook.n_action].id = gamebook.n_action + 1;
			gamebook.action[gamebook.n_action].choices = NULL;
			gamebook.action[gamebook.n_action].n_choice = 0;

			node_child = ((xmlNodePtr)node->children);

			while(node_child)
			{
				if (node_child->type == XML_ELEMENT_NODE && node_child->children->type == XML_TEXT_NODE)
				{
					if( !strcmp(xmlStrdup(node_child->name), "text") )
					{
						gamebook.action[gamebook.n_action].text = strreplace(xmlStrdup( node_child->children->content ) , "$user", user_cfg.name );
					} else {
						if(!gamebook.action[gamebook.n_action].choices)
							gamebook.action[gamebook.n_action].choices = malloc( sizeof(struct gbook_choice) * CH_BLOCK );

						if(gamebook.action[gamebook.n_action].n_choice % CH_BLOCK)
							gamebook.action[gamebook.n_action].choices = realloc( gamebook.action[gamebook.n_action].choices, 
									( CH_BLOCK + gamebook.action[gamebook.n_action].n_choice ) * sizeof(struct gbook_choice) );

						gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].text = strreplace( xmlStrdup( node_child->children->content ), "$user", user_cfg.name );

						gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].require = \
						gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].gain = NULL;
						gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].life = \
						gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].money = 0;

						prop = node_child->properties;

						while( prop )
						{
							if( !strcmp( xmlStrdup( prop->name) , "link" ) )
							{
								gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].link = _atoi(xmlStrdup( prop->children->content ));
							} else if (!strcmp( xmlStrdup( prop->name ), "require" ))
							{
								gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].require = xmlStrdup( prop->children->content );
							} else if (!strcmp( xmlStrdup( prop->name ), "gain" ))
							{
								gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].gain = xmlStrdup( prop->children->content );
							} else if (!strcmp( xmlStrdup( prop->name ), "life" ))
							{
								gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].life = _atoi(xmlStrdup( prop->children->content ));
							} else if (!strcmp( xmlStrdup( prop->name ), "money" ))
							{
								gamebook.action[gamebook.n_action].choices[gamebook.action[gamebook.n_action].n_choice].money = _atoi(xmlStrdup( prop->children->content ));
							}

							prop = prop->next;
						}

						#ifdef _DEBUG
							printf("%s -> \"%s\"\n", xmlStrdup(node_child->name), xmlStrdup(node_child->children->content) );

							if ( node_child->properties )
								printf("( %s = %s )\n", xmlStrdup( node_child->properties->name ), xmlStrdup( node_child->properties->children->content ) );
						#endif

						gamebook.action[gamebook.n_action].n_choice++;
					}
				}

				node_child = node_child->next;
			}

			gamebook.n_action++;
		}

		node = node->next;
	}
}

void
saveConfigs( char* file, int action )
{
	FILE* fp;
	int i;

	if( !(fp = fopen( file, "w") ))
	{
		perror("fopen");
		exit( 1 );
	}

	fprintf( fp, "<?xml version=\"1.0\"?>\n" );
	fprintf( fp, "<configs>\n" );
	fprintf( fp, "\t<name>%s</name>\n", user_cfg.name );
	fprintf( fp, "\t<money>%d</money>\n", user_cfg.money );
	fprintf( fp, "\t<life>%d</life>\n", user_cfg.lifep );

	for(i = 0;i < user_cfg.n_object;i++)
		fprintf( fp, "\t<object>%s</object>\n", user_cfg.objects[i] );

	fprintf( fp, "\t<story>%d</story>\n", action );

	fprintf( fp, "</configs>\n" );

	fclose( fp );
}	

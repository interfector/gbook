#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <gbook.h>

struct gbook_engine gamebook = { NULL, 0 };
struct user_configs user_cfg = { NULL, 0, 100, NULL, 0 };

struct gbook_merchant merchant = { NULL, 0 };

char* cfg_file = NULL;
int i = 0;

int
menu( struct gbook_choice* choices, int size )
{
	int i;
	char line[BUFSIZ];

	while(1)
	{
		for(i = 0;i < size;i++)
			printf("%d) %s\n", i + 1, choices[i].text );

		if(!fgets(line, BUFSIZ, stdin))
			return -1;

		sscanf( line, "%d", &i );

		if( i >= 1 && i <= size )
			return i - 1;
	}

	return -1;
}

int
menu_merch( struct gbook_object* objs, char** names, int size )
{
	int i;
	char line[BUFSIZ];

	while(1)
	{
		for(i = 0;i < size;i++)
		{
			if( objs )
				printf("%d) %s ( %d money )\n", i + 1, objs[i].name, objs[i].money );
			else
				printf("%d) %s\n", i + 1, names[i] );
		}

		if(!fgets(line, BUFSIZ, stdin))
			return -1;

		sscanf( line, "%d", &i );

		if( i >= 1 && i <= size )
			return i - 1;
	}

	return -1;
}

char*
getLine(char *text)
{
	char* line = malloc( 256 );

	printf( "%s", text);

	if(!fgets( line, 256, stdin ))
	{
		perror("fgets");

		exit(1);
	}

	line[strlen(line)-1] = 0;

	return line;
}

int
search( char* obj, struct user_configs conf )
{
	int i;

	for(i = 0;i < conf.n_object;i++)
		if(!strcmp(obj, conf.objects[i] ))
			return 1;

	return 0;
}

void
handle( int dummy )
{
	saveConfigs( cfg_file, i );

	exit(0);
}

char**
removeFrom( char** objs, int size, char* name )
{
	int i;
	char** tmp;
	char** ptr;

	for(i = 0;i < size;i++)
	{
		if(!strcmp(objs[i],name))
		{
			tmp = malloc( sizeof(char*) * (size - i));
			memcpy( tmp, objs + i + 1, (size - i) * sizeof(char*) );

			memcpy( &objs[i], tmp, (size - i) );
		}
	}
}

struct gbook_object*
search_obj( struct gbook_object* objs, char* name, int size )
{
	int i;

	for(i = 0;i < size;i++)
	{
		if(!strcmp(objs[i].name,name))
			return &objs[i];
	}

	return NULL;
}

void
go_merchant()
{
	char* action[] = { "Buy", "Sell", "Inventory", "Exit" };
	struct gbook_object* tosold = malloc( sizeof(struct gbook_object) * user_cfg.n_object );
	int n_tobj = 0;
	int i = 0, j = 0, k = 0;

	loadMerchant( "./merchant.xml" );

	for(k = 0;k < user_cfg.n_object;k++)
	{
		if( search_obj( merchant.objs, user_cfg.objects[k], merchant.n_obj ))
		{
			tosold[n_tobj].name = strdup( user_cfg.objects[k] );
			tosold[n_tobj++].money = (search_obj( merchant.objs, user_cfg.objects[k], merchant.n_obj))->money;
		}
	}

	while(i != 3)
	{
		i = menu_merch( NULL, action, 4 );

		if( i == 3 )
			return;

		if( i == 0 )
		{
			j = menu_merch( merchant.objs, NULL, merchant.n_obj );

			if( user_cfg.money < merchant.objs[j].money )
			{
				printf("You haven't enough money!\n"  );

				continue;
			} else {
				if(!user_cfg.objects)
					user_cfg.objects = malloc( CH_BLOCK * sizeof(struct gbook_object) );

				if( user_cfg.n_object % CH_BLOCK )
					user_cfg.objects = realloc(user_cfg.objects, (CH_BLOCK + user_cfg.n_object) * sizeof(char*) );

				user_cfg.objects[ user_cfg.n_object++ ] = strdup( merchant.objs[j].name );

				user_cfg.money -= merchant.objs[j].money;

				printf("Bought %s. Now you have %d money.\n", merchant.objs[j].name, user_cfg.money );
			}
		} else if ( i == 1 )
		{
			if( !user_cfg.n_object )
			{
				printf("Nothing to sell!\n" );

				continue;
			}

			j = menu_merch( tosold, NULL, n_tobj );

			user_cfg.money += (tosold[j].money - (tosold[j].money * 20 / 100 ));

			if( j + 1 != n_tobj )
				memcpy( &tosold[j] , &tosold[j + 1], sizeof(struct gbook_object) * (n_tobj - j - 1) );

			n_tobj--;

			removeFrom( user_cfg.objects, user_cfg.n_object, tosold[j].name );

			user_cfg.n_object--;

			printf("Sold %s. Now you have %d money.\n", tosold[j].name, user_cfg.money );
		} else if ( i == 2 )
		{
			for(k = 0;k < user_cfg.n_object;k++)
			{
				printf("%s ", user_cfg.objects[k] );

				if( search_obj( merchant.objs, user_cfg.objects[k], merchant.n_obj ))
					printf("( %d )", (search_obj(merchant.objs, user_cfg.objects[k], merchant.n_obj))->money );

				printf("\n");
			}
		}
	}
}

int
main(int argc,char **argv)
{
	int j = 0;

	signal( SIGINT, handle );

	if(argv[1])
	{
		loadProfile(argv[1]);

		cfg_file = strdup(argv[1]);
	} else {
		cfg_file = strdup("./.config.xml");

		if( !access( "./.config.xml", F_OK ) )
			loadProfile("./.config.xml");
		else
			user_cfg.name = getLine("Insert your name: ");
	}

	loadBook("./story.xml");

#ifdef _DEBUG
	for(i = 0;i < gamebook.n_action;i++)
	{
		printf("%d => %s\n", gamebook.action[ i ].id, gamebook.action[ i ].text );

		for(j = 0;j < gamebook.action[ i ].n_choice;j++)
		{
			printf("\t[ %d ]( life: %d, money: %d ) => %s\n", gamebook.action[ i ].choices[ j ].link, gamebook.action[ i ].choices[ j ].life, gamebook.action [ i ].choices[ j ].money, gamebook.action[ i ].choices[ j ].text );

		}
	}

	i = j = 0;
#endif

	while( 1 )
	{
		if( user_cfg.lifep <= 0 )
		{
			printf("You were killed! GAME OVER!\n");

			exit( 0 );
		}

		printf("%s\n", gamebook.action[ i ].text );

		j = menu( gamebook.action[ i ].choices, gamebook.action[ i ].n_choice );

		if( gamebook.action[ i ].choices[ j ].require )
		{
			if( !search( gamebook.action[ i ].choices[ j ].require, user_cfg ) )
			{
				printf("You haven't \'%s\'!\n", gamebook.action[ i ].choices[ j ].require );

				continue;
			}
		}
		
		if( gamebook.action[ i ].choices[ j ].gain )
		{
			if( search( gamebook.action[ i ].choices[ j ].gain, user_cfg ) )
			{
				printf("You already have \'%s\'!\n", gamebook.action[ i ].choices[ j ].gain );
			} else {
				if( !user_cfg.objects )
					user_cfg.objects = malloc( CH_BLOCK * sizeof(char*) );

				if( user_cfg.n_object % CH_BLOCK )
					user_cfg.objects = realloc(user_cfg.objects, (CH_BLOCK + user_cfg.n_object) * sizeof(char*) );

				user_cfg.objects[ user_cfg.n_object++ ] = strdup( gamebook.action[ i ].choices[ j ].gain );

				printf("You took \'%s\'!\n", gamebook.action[ i ].choices[ j ].gain );
			}
		}

		if( gamebook.action[ i ].choices[ j ].life )
		{
			if( gamebook.action[ i ].choices[ j ].life < 0 )
				printf("You lose %d life points. ", abs( gamebook.action[ i ].choices[ j ].life ) );
			else
				printf("You gain %d life points. ", abs( gamebook.action[ i ].choices[ j ].life ) );

			user_cfg.lifep += gamebook.action[ i ].choices[ j ].life;

			printf("Now you have %d life points.\n", user_cfg.lifep );
		}

		if( gamebook.action[ i ].choices[ j ].money )
		{
			if( gamebook.action[ i ].choices[ j ].money < 0 && abs(gamebook.action[ i ].choices[ j ].money) > user_cfg.money )
			{
				printf("You haven't enough money!\n"  );

				continue;
			}

			if( gamebook.action[ i ].choices[ j ].money < 0 )
				printf("You pay %d money.", abs( gamebook.action[ i ].choices[ j ].money ) );
			else
				printf("You gain %d money.", abs( gamebook.action[ i ].choices[ j ].money ) );
			
			user_cfg.money += gamebook.action[ i ].choices[ j ].money;

			printf("Now you have %d money.\n", user_cfg.money );
		}

		if( gamebook.action[ i ].choices[ j ].link == -1 )
		{
			printf("THE END!\n");

			i = 0;

			saveConfigs( cfg_file, i );

			exit( 0 );
		} else if( gamebook.action[ i ].choices[ j ].link == -2 )
		{
			printf("GAME OVER!\n");

			i = 0;

			saveConfigs( cfg_file, i );

			exit( 0 );
		} else if( gamebook.action[ i ].choices[ j ].link == -3 )
		{
			go_merchant();

			continue;
		}

		i = gamebook.action[ i ].choices[ j ].link - 1;
	}

	return 0;
}

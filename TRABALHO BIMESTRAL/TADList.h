#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct lista
{
	char token[50];
	struct lista *prox;
};
typedef struct lista ListEnc;

void InitLista(ListEnc**Lista)
{
	*Lista = NULL;
}

void ListEnqueue(ListEnc**Lista, char token[50])
{
	ListEnc *aux;
	ListEnc *novo = (ListEnc *)malloc(sizeof(ListEnc));
	strcpy(novo->token, token);
	novo->prox = NULL;
	
	if(*Lista == NULL)
		*Lista = novo;
	else
	{
		aux = *Lista;
		while(aux->prox != NULL)
			aux = aux->prox;
			
		aux->prox = novo;
	}
}

void ListDequeue(ListEnc **Lista, char *comando)
{
    ListEnc *aux;
    
    if(*Lista != NULL)
    {
        aux = *Lista;
        *Lista = (*Lista)->prox;
        strcpy(comando, aux->token);
        free(aux);
    }
    else
        comando[0] = '\0';
}

void ListExibe(ListEnc *Lista)
{
	while(Lista != NULL)
	{
		printf("%s\n", Lista->token);
		Lista = Lista->prox;
	}
}

void CriarListaComandos(ListEnc**Lista, char nomeArq[100])
{
	char linha[100], token[50];
	int pos, j;
	FILE *ptrArq = fopen(nomeArq, "r");
	
	while(fgets(linha, sizeof(linha), ptrArq) != NULL)
	{
		pos = 0;
		while(pos < strlen(linha))
		{
			j = 0;	
			while(linha[pos] != '\0' && (linha[pos] == ' ' || linha[pos] == ';' || linha[pos] == '\n' || linha[pos] == '\r'))
				pos++;
			
			while(linha[pos] != '\0' && linha[pos] != ' ' && linha[pos] != '(' && linha[pos] != ')' && linha[pos] != ';' && linha[pos] != '\n' && linha[pos] != '\r' && linha[pos] != ',')
				token[j++] = linha[pos++];
				
			if(j > 0)
			{
				token[j] = '\0';
				ListEnqueue(&*Lista, token);
			}
			else if(linha[pos] == '(' || linha[pos] == ')' || linha[pos] == ',')
			{
				token[j++] = linha[pos++];
				token[j] = '\0';
				ListEnqueue(&*Lista, token);
			}
		}
	}
	
	fclose(ptrArq);
}

void CriarListaUsuario(ListEnc** Lista, char comando[100])
{
    char token[50];
    int pos, j;

    if (comando[0] != '\0')
    {
        pos = 0;
        while (pos < strlen(comando))
        {
            j = 0;

            while (comando[pos] != '\0' && (comando[pos] == ' ' || comando[pos] == ';' || comando[pos] == '\n' || comando[pos] == '\r'))
                pos++;

            while (comando[pos] != '\0' && comando[pos] != ' ' && comando[pos] != '(' && comando[pos] != ')' && comando[pos] != ';' && 
				   comando[pos] != '\n' && comando[pos] != '\r' && comando[pos] != ',' && comando[pos] != '=' && comando[pos] != '.' && 
				   comando[pos] != '*' && comando[pos] != '<' && comando[pos] != '>' && comando[pos] != '!')
            {
                token[j++] = comando[pos++];
            }
            
            if (j > 0)
            {
                token[j] = '\0';
                ListEnqueue(&*Lista, token);
            }
            else if (comando[pos] == '<' || comando[pos] == '>' || comando[pos] == '!')
            {
                token[j++] = comando[pos++];

                if (comando[pos] == '=')
                    token[j++] = comando[pos++];

                token[j] = '\0';
                ListEnqueue(&*Lista, token);
            }
            else if (comando[pos] == '(' || comando[pos] == ')' || comando[pos] == ',' || comando[pos] == '=' || comando[pos] == '.' || comando[pos] == '*')
            {
                token[j++] = comando[pos++];
                token[j] = '\0';
                ListEnqueue(&*Lista, token);
            }
        }
    }
}

ListEnc* buscaWhere(ListEnc**Lista)
{
	ListEnc *aux, *aux2;
	aux = *Lista;
	
	if (aux == NULL) 
        return aux;
        
	if (stricmp(aux->token, "WHERE") == 0)
    {
        aux2 = aux->prox; /* Pega nas condições à frente do WHERE */
        *Lista = NULL;    /* Esvazia a lista principal */
        return aux2;
    }
    
	while(aux != NULL)
	{
		while(aux->prox != NULL && stricmp(aux->prox->token,"WHERE") != 0)
			aux = aux->prox;
		
		if(aux->prox != NULL && stricmp(aux->prox->token,"WHERE") == 0)
		{
			//aux2 para na primeira condição do WHERE
			aux2 = aux->prox->prox;
			aux->prox = NULL;
			return aux2;
		}
		else
		{
			aux = NULL;
			return aux;
		}
	}
}


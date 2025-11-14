#include <stdio.h>
#include <limits.h>

#define MAX 20

void dijkstra(int n, int graph[MAX][MAX], int src) {
    int dist[MAX];       // shortest distance from source
    int visited[MAX];    // visited vertices
    int prev[MAX];       // previous node in path

    for(int i=0;i<n;i++){
        dist[i] = INT_MAX;
        visited[i] = 0;
        prev[i] = -1;
    }

    dist[src] = 0;

    for(int count=0; count<n-1; count++){
        // Find minimum distance vertex from unvisited
        int min = INT_MAX, u=-1;
        for(int v=0; v<n; v++){
            if(!visited[v] && dist[v]<=min){
                min = dist[v];
                u = v;
            }
        }

        if(u==-1) break; // no reachable vertex
        visited[u] = 1;

        // Update distances of adjacent vertices
        for(int v=0; v<n; v++){
            if(!visited[v] && graph[u][v]!=0 &&
               dist[u]!=INT_MAX && dist[u]+graph[u][v]<dist[v]){
                dist[v] = dist[u]+graph[u][v];
                prev[v] = u;
            }
        }
    }

    // Print shortest paths
    printf("Vertex\tDistance from Source\tPath\n");
    for(int i=0;i<n;i++){
        printf("%d\t%d\t\t\t", i, dist[i]);
        if(dist[i]==INT_MAX) { printf("-\n"); continue; }

        // print path
        int path[MAX], count=0, temp=i;
        while(temp!=-1){
            path[count++] = temp;
            temp = prev[temp];
        }
        for(int j=count-1;j>=0;j--){
            printf("%d", path[j]);
            if(j!=0) printf(" -> ");
        }
        printf("\n");
    }
}

int main(){
    int n;
    int graph[MAX][MAX];

    printf("Enter number of vertices (max %d): ", MAX);
    scanf("%d",&n);

    printf("Enter adjacency matrix (0 if no edge):\n");
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            scanf("%d",&graph[i][j]);
        }
    }

    int src;
    printf("Enter source vertex: ");
    scanf("%d",&src);

    dijkstra(n, graph, src);

    return 0;
}

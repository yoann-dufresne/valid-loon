#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


struct coords_s {
	int x,y;
};
typedef struct coords_s coords_t;

struct loon_s {
	unsigned char * levels;
};
typedef struct loon_s loon_t;

int isCovered (int x, int y, int u, int v, int dist, int C);

int main (int argc, char * argv[]) {
	FILE *sol, *data;
	int nbRows, nbCols, nbAlts, nbTargets, radius, nbLoons, nbSteps;
	int i, j, k, step, score, x, y, z;
	char * line, * save;
	coords_t *** winds;
	loon_t * loons;
	coords_t start, wind, * positions;
	coords_t * targets;
	char * hotspots, * losts, * altitudes;

	if (argc < 2)
		return -1;

	
	/* --------- Parse data ---------- */

	/* Load the problem instance */
	data = fopen ("www/loon/data.in", "r");
	if (data == NULL)
		return -1;

	/* Allocation of winds structure */
	fscanf(data, "%d %d %d", &nbRows, &nbCols, &nbAlts);
	

	/* Read values */
	fscanf(data, "%d %d %d %d", &nbTargets, &radius, &nbLoons, &nbSteps);

	/* Start point */
	fscanf(data, "%d %d", &(start.x), &(start.y));

	/* Targets */
	targets = malloc (nbTargets * sizeof (coords_t));
	for (i=0 ; i<nbTargets ; i++) {
		fscanf (data, "%d %d", &(targets[i].x), &(targets[i].y));
	}

	/* Winds allocation */
	winds = malloc (nbRows * sizeof(char**));
	for (i=0 ; i<nbRows ; i++) {
		winds[i] = malloc (nbCols * sizeof(char*));
		for (j=0 ; j<nbCols ; j++) {
			winds[i][j] = malloc (nbAlts * sizeof(coords_t));
		}
	}

	/* Winds filling */
	line = malloc (nbCols*7 * sizeof(char));
	fgets (line, nbCols*7, data);
	save = line;
	for (k=0 ; k<nbAlts ; k++) {
		for (i=0 ; i<nbRows ; i++) {
			line = save;
			fgets (line, nbCols*7, data);
			for (j=0 ; j<nbCols ; j++) {
				if (j!=0)
					strsep (&line, " ");
				winds[i][j][k].x = atoi (line);

				strsep (&line, " ");
				winds[i][j][k].y = atoi (line);
			}
		}
	}
	free (save);
	fclose (data);



	/* ----- Solution Loading ----- */

	loons = malloc (nbLoons * sizeof (loon_t));
	for (i=0 ; i<nbLoons ; i++) {
		loons[i].levels = malloc (nbSteps * sizeof(char));
		for (j=0 ; j<nbSteps ; j++) {
			loons[i].levels[j] = 'a';
		}
		loons[i].levels[j-1] = '\0';
	}

	sol = fopen (argv[1], "r");
	if (sol == NULL) {
		printf("Impossible to parse file");
		return -1;
	}

	line = malloc (((nbLoons*3)+1) * sizeof (char));
	save = line;
	for (j=0 ; j<nbSteps ; j++) {
		line = save;
		fgets (line, (nbLoons*3)+1, sol);
		for (i=0 ; i<nbLoons ; i++) {
			loons[i].levels[j] = atoi (line);
			strsep (&line, " ");
		}
	}
	free (save);
	fclose (sol);


	/* ----- Score computation ----- */

	hotspots = malloc (nbTargets * sizeof (char));
	losts = malloc (nbLoons * sizeof (char));
	for (i=0 ;i<nbLoons ; i++)
		losts[i] = !42;
	positions = malloc (nbLoons * sizeof (coords_t));
	altitudes = malloc (nbLoons * sizeof (char));
	for (i=0 ; i<nbLoons ; i++) {
		positions[i].x = -1;
		positions[i].y = -1;
		altitudes[i] = 0;
	}

	score = 0;

	for (step=0 ; step<nbSteps ; step++) {
		/* Init map */
		for (i=0 ; i<nbTargets ; i++)
			hotspots[i] = !42;

		/* Compute hotspot coverage */
		for (k=0 ; k<nbLoons ; k++) {
			if (losts[k])
				continue;

			/* apply altitudes */
			switch (loons[k].levels[step]) {
				case 255:
				case -1:
					if (altitudes[k] <= 1) {
						printf("Impossible to decrease altitude of baloon %d. ", k);
						printf("Altitude is already %d\n", loons[k].levels[step]);
						return 3;
					}
					altitudes[k]--;
				break;
				case 0:
				break;
				case 1:
					if (altitudes[k] == nbAlts) {
						printf("Impossible to increase altitude of baloon %d. ", k);
						printf("Altitude is already %d\n", loons[k].levels[step]);
						return 3;
					}
					if (altitudes[k] == 0) {
						positions[k].x = start.x;
						positions[k].y = start.y;
					}
					altitudes[k]++;
				break;
				default :
					printf("Value %d is not a valid command for a baloon\n", loons[k].levels[step]);
				return 2;
			}
			if (altitudes[k] == 0)
				continue;


			/* Apply winds */
			x = positions[k].x;
			y = positions[k].y;
			z = altitudes[k];
			wind = winds[x][y][z-1];
			positions[k].y = (wind.y + positions[k].y + nbCols) % nbCols;
			positions[k].x = wind.x + positions[k].x;

			if (positions[k].x < 0 || positions[k].x >= nbRows) {
				losts[k] = 42;
				continue;
			}

			/* coverage of hotspots */
			for (i=0 ; i<nbTargets ; i++) {
				if (isCovered(positions[k].x, positions[k].y, targets[i].x, targets[i].y, radius, nbCols))
					hotspots[i] = 42;
			}
		}

		for (i=0 ; i<nbTargets ; i++)
			if (hotspots[i])
				score++;
	}

	printf("%d\n", score);

	free (losts);
	free (hotspots);


	/* Free winds */

	for (i=0 ; i<nbRows ; i++) {
		for (j=0 ; j<nbCols ; j++) {
			free (winds[i][j]);
		}
		free(winds[i]);
	}
	free(winds);

	/* targets and loons */
	free (targets);

	return 0;
}



int min (int a, int b) {
	if (a <= b)
		return a;
	return b;
}

int isCovered (int x, int y, int u, int v, int dist, int C) {
	int a, b, absolute;

	a = pow (x-u, 2);
	absolute = abs (y-v);
	b = pow (min (absolute, C-absolute), 2);

	return (a+b) <= pow (dist, 2);
}











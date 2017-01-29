int main(){
	int x[4*1024] = {0};
	int i = 0;
	while(1){
		x[i++] += 1;
		if (i==4096)
			i = 0;

	}
	return 0;
}

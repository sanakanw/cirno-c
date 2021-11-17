
fn main()
{
  i32 i, p, n, prime[8];
	
	n = 8;
	
	i = 0;
	while (i < n) {
		prime[i] = 1;
		i = i + 1;
	}
	
	p = 2;
	while (p * p < n) {
		if (prime[p] > 0) {
			i = p * p;
			while (i < n) {
				prime[i] = 0;
				i = i + p;
			}
		}
		
		p = p + 1;
	}
}

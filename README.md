wtmpfilter
==========

Filters users from /var/log/wtmp. Usage:

	./wtmpfilter shadyuser < /var/log/wtmp | sponge /var/log/wtmp

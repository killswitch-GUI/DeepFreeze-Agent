# Soft vs Hard Indicators

##  Indicators of Compromise

**Indicator of compromise \(IOC\)** — in computer forensics is an artifact observed on a network or in an operating system that with high confidence indicates a computer intrusion.

Typical IOCs are virus signatures and IP addresses, MD5 hashes of malware files or URLs or domain names of botnet command and control servers. After IOCs have been identified in a process of incident response and computer forensics, they can be used for early detection of future attack attempts using intrusion detection systems and antivirus software.

## Soft vs Hard Indicators

Soft Indicators of Compromise are indicators that are based upon artifacts that can be changed by attacker. Hard Indicators of Compromise are indicators that are either very difficult or impossible to be changed by an attacker

## Ten Common Indicators of Compromise

Unusual account behaviors, strange network patterns, unexplained configuration changes, and odd files on systems can all point to a potential breach

In the quest to detect data breaches more quickly, indicators of compromise can act as important breadcrumbs for security pros watching their IT environments. Unusual activity on the network or odd clues on systems can frequently help organizations spot attacker activity on systems more quickly so that they can either prevent an eventual breach from happening -- or at least stop it in its earliest stages.

According to the experts, here are some key indicators of compromise to monitor \(in no particular order\):

### 1. Unusual Outbound Network Traffic

Perhaps one of the biggest telltale signs that something is amiss is when IT spots unusual traffic patterns leaving the network.

"A common misperception is that traffic inside the network is secure," says Sam Erdheim, senior security strategist for AlgoSec. "Look for suspicious traffic leaving the network. It's not just about what comes into your network; it's about outbound traffic as well."

Considering that the chances of keeping an attacker out of a network are difficult in the face of modern attacks, outbound indicators may be much easier to monitor, says Geoff Webb, director of solution strategy for NetIQ.

"So the best approach is to watch for activity within the network and to look for traffic leaving your perimeter," he says. "Compromised systems will often call home to command-and-control servers, and this traffic may be visible before any real damage is done."

### 2. Anomalies In Privileged User Account Activity

The name of the game for a well-orchestrated attack is for attackers to either escalate privileges of accounts they've already compromised or to use that compromise to leapfrog into other accounts with higher privileges. Keeping tabs on unusual account behavior from privileged accounts not only watches out for insider attacks, but also account takeover.

"Changes in the behavior of privileged users can indicate that the user account in question is being used by someone else to establish a beachhead in your network," Webb says. "Watching for changes -- such as time of activity, systems accessed, type or volume of information accessed -- will provide early indication of a breach."

### 3. Geographical Irregularities

Whether through a privileged account or not, geographical irregularities in log-ins and access patterns can provide good evidence that attackers are pulling strings from far away. For example, traffic between countries that a company doesn't do business with offers reason for pause.

"Connections to countries that a company would normally not be conducting business with \[indicates\] sensitive data could be siphoned to another country," says Dodi Glenn, director of security content management for ThreatTrack Security.

Similarly, when one account logs in within a short period of time from different IPs around the world, that's a good indication of trouble.

"As to data-breach clues, one of the most useful bits I've found is logs showing an account logging in from multiple IPs in a short time period, particularly when paired with geolocation tagging," says Benjamin Caudill, principal consultant for Rhino Security. "More often than not, this is a symptom of an attacker using a compromised set of credentials to log into confidential systems."

### 4. Other Log-In Red Flags

Log-in irregularities and failures can provide excellent clues of network and system probing by attackers.

Similarly, attempted and successful log-in activity after hours can provide clues that it isn't really an employee who is accessing data.

"If you see John in accounting logging onto the system after work hours and trying to access files for which he is not authorized, this bears investigation," says A.N. Ananth, CEO of EventTracker.

### 5. Swells In Database Read Volume

Once an attacker has made it into the crown jewels and seeks to exfiltrate information, there will be signs that someone has been mucking about data stores. One of them is a spike in database read volume, says Kyle Adams, chief software architect for Junos WebApp Secure at Juniper Networks.

"When the attacker attempts to extract the full credit card database, it will generate an enormous amount of read volume, which will be way higher than you would normally see for reads on the credit card tables," he says.

### 6. HTML Response Sizes

Adams also says that if attackers use SQL injection to extract data through a Web application, the requests issued by them will usually have a larger HTML response size than a normal request.

"For example, if the attacker extracts the full credit card database, then a single response for that attacker might be 20 to 50 MB, where a normal response is only 200 KB," he says.

### 7. Large Numbers Of Requests For The Same File

It takes a lot of trial and error to compromise a site -- attackers have to keep trying different exploits to find ones that stick. And when they find signs that an exploit might be successful, they'll frequently use different permutations to launch it.

"So while the URL they are attacking will change on each request, the actual filename portion will probably stay the same," Adams says. "So you might see a single user or IP making 500 requests for 'join.php,' when normally a single IP or user would only request that page a few times max."

### 8. Mismatched Port-Application Traffic

Attackers often take advantage of obscure ports to get around more simple Web filtering techniques. So if an application is using an unusual port, it could be sign of command-and-control traffic masquerading as "normal" application behavior.

"We have noticed several instances of infected hosts sending C&C communications masked as DNS requests over port 80," says Tom Gorup, SOC analyst for Rook Consulting. "At first glance, these requests may appear to be standard DNS queries; however, it is not until you actually look at those queries that you see the traffic going across a nonstandard port. "

\[Your organization's been breached. Now what? See Establishing The New Normal After A Breach.\]

### 9. Suspicious Registry Or System File Changes

One of the ways malware writers establish persistence within an infected host is through registry changes.

"Creating a baseline is the most important part when dealing with registry-based IOCs," Gorup says. "Defining what a clean registry is supposed to contain essentially creates the filter against which you will compare your hosts. Monitoring and alerting on changes that deviate outside the bounds of the clean 'template' can drastically increase security team response time."

Similarly, many attackers will leave behind signs that they've tampered with a host in system files and configurations, says Webb, who has seen organizations more quickly identify compromised systems by looking for these kinds of changes.

"What can happen is that the attacker will install packet-sniffing software to harvest credit card data as it moves around the network," he says. "The attacker targets a system that can watch the network traffic, then installs the harvesting tool. While the chances of catching the specific harvesting tool are slim -- because they will be targeted and probably not seen before -- there is a good chance to catch the changes to the system that houses the harvesting tool."

### 10. DNS Request Anomalies

According to Wade Williamson, senior security analyst for Palo Alto Networks, one of the most effective red flags an organization can look for are telltale patterns left by malicious DNS queries.

"Command-and-control traffic is often the most important traffic to an attacker because it allows them ongoing management of the attack and it needs to be secure so that security professionals can't easily take it over," he says. "The unique patterns of this traffic can be recognized and is a very standard approach to identifying a compromise."

Gorup agrees that DNS exfiltration can be "extremely loud."

"Seeing a large spike in DNS requests from a specific host can serve as a good indicator of potentially suspect activity," he says. "Watching for patterns of DNS requests to external hosts, compared against geoIP and reputation data, and implementing appropriate filtering can help mitigate C&C over DNS."


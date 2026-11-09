# About the Datasets

We conduct our experiments using two public datasets.  

For each dataset, we provide a demo file in this repository to help verify that the code runs correctly. Dataset references and access links are provided below.

---

## CAIDA Anonymized Internet Traces

- **Description**: This dataset contains anonymized Internet traffic traces collected by **CAIDA** in 2019. Each packet is identified by its source and destination IP addresses.
- **Demo File**: A small demo trace (txt format) is included in this repository for testing. We download the raw CAIDA data, parse the raw traffic and convert it into plain text (txt) format.
- **Full Dataset**: Available upon request from the official CAIDA website:  
  https://catalog.caida.org/dataset/passive_2019_pcap

---

## Frequent Itemset Mining Dataset

- **Description**: Our experiments use the dataset from the Frequent Itemset Mining Implementations (FIMI) repository. 
- **Demo File**: We provide `kosarak.dat`, which contains anonymized click-stream data from a Hungarian online news portal.
- **Reference**: Bart Goethals and Mohammed J. Zaki. 2004. Advances in frequent itemset mining implementations: report on FIMI'03. *ACM SIGKDD Explorations Newsletter* 6, 1 (June 2004), 109-117. 
- **Dataset Access**: Dataset descriptions and download links are also listed on the https://www.cs.rpi.edu/~zaki/Workshops/FIMI/data/
- **Historical Download Source**: http://fimi.uantwerpen.be/data/ was the original download location used for our experiments. We retain it for provenance; if it is unavailable, consult the repository mirror and references provided here.



---

> ⚠️ **Note**: The demo datasets provided in this repository are for research and code testing only. Please comply with the official dataset usage policies and cite their sources when using the full datasets.

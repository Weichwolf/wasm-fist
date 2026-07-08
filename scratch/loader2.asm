
scratch/loader2.bin:     file format binary


Disassembly of section .data:

00005780 <.data>:
    5780:	cd 31                	int    $0x31
    5782:	b8 01 4c             	mov    $0x4c01,%ax
    5785:	cd 21                	int    $0x21
    5787:	66 60                	pushal
    5789:	1e                   	push   %ds
    578a:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
    578f:	e8 19 00             	call   0x57ab
    5792:	1f                   	pop    %ds
    5793:	66 61                	popal
    5795:	c3                   	ret
    5796:	be 0c 06             	mov    $0x60c,%si
    5799:	b3 00                	mov    $0x0,%bl
    579b:	b8 02 02             	mov    $0x202,%ax
    579e:	e8 51 00             	call   0x57f2
    57a1:	83 c6 06             	add    $0x6,%si
    57a4:	fe c3                	inc    %bl
    57a6:	80 fb 0f             	cmp    $0xf,%bl
    57a9:	76 f0                	jbe    0x579b
    57ab:	be 6c 06             	mov    $0x66c,%si
    57ae:	b4 02                	mov    $0x2,%ah
    57b0:	2e 8b 1e 16 10       	mov    %cs:0x1016,%bx
    57b5:	66 0f 03 db          	lsl    %ebx,%ebx
    57b9:	66 d1 d3             	rcl    $1,%ebx
    57bc:	0f 83 2d 00          	jae    0x57ed
    57c0:	b3 10                	mov    $0x10,%bl
    57c2:	b0 04                	mov    $0x4,%al
    57c4:	e8 2b 00             	call   0x57f2
    57c7:	83 c6 06             	add    $0x6,%si
    57ca:	fe c3                	inc    %bl
    57cc:	80 fb 15             	cmp    $0x15,%bl
    57cf:	76 f1                	jbe    0x57c2
    57d1:	b3 21                	mov    $0x21,%bl
    57d3:	b0 04                	mov    $0x4,%al
    57d5:	e8 1a 00             	call   0x57f2
    57d8:	83 c6 06             	add    $0x6,%si
    57db:	b3 23                	mov    $0x23,%bl
    57dd:	b0 04                	mov    $0x4,%al
    57df:	e8 10 00             	call   0x57f2
    57e2:	83 c6 06             	add    $0x6,%si
    57e5:	b3 33                	mov    $0x33,%bl
    57e7:	b0 04                	mov    $0x4,%al
    57e9:	e8 06 00             	call   0x57f2
    57ec:	c3                   	ret
    57ed:	8d 74 24             	lea    0x24(%si),%si
    57f0:	eb df                	jmp    0x57d1
    57f2:	cd 31                	int    $0x31
    57f4:	0f 82 66 ff          	jb     0x575e
    57f8:	66 87 14             	xchg   %edx,(%si)
    57fb:	87 4c 04             	xchg   %cx,0x4(%si)
    57fe:	fe c0                	inc    %al
    5800:	cd 31                	int    $0x31
    5802:	0f 82 58 ff          	jb     0x575e
    5806:	c3                   	ret
    5807:	e8 88 d2             	call   0x2a92
    580a:	66 57                	push   %edi
    580c:	67 66 8d 64 24 d0    	lea    -0x30(%esp),%esp
    5812:	b8 00 05             	mov    $0x500,%ax
    5815:	66 8b fc             	mov    %esp,%edi
    5818:	16                   	push   %ss
    5819:	07                   	pop    %es
    581a:	cd 31                	int    $0x31
    581c:	66 58                	pop    %eax
    581e:	67 66 8d 64 24 2c    	lea    0x2c(%esp),%esp
    5824:	66 5f                	pop    %edi
    5826:	66 a3 d8 04          	mov    %eax,0x4d8
    582a:	e8 8c d2             	call   0x2ab9
    582d:	c3                   	ret
	...
    583a:	e9 81 0b             	jmp    0x63be
    583d:	00 00                	add    %al,(%bx,%si)
    583f:	4b                   	dec    %bx
    5840:	4b                   	dec    %bx
    5841:	4b                   	dec    %bx
    5842:	4b                   	dec    %bx
    5843:	4b                   	dec    %bx
    5844:	4b                   	dec    %bx
    5845:	4b                   	dec    %bx
    5846:	4b                   	dec    %bx
    5847:	4b                   	dec    %bx
    5848:	4b                   	dec    %bx
    5849:	4b                   	dec    %bx
    584a:	4b                   	dec    %bx
    584b:	4b                   	dec    %bx
    584c:	4b                   	dec    %bx
    584d:	4b                   	dec    %bx
    584e:	4b                   	dec    %bx
    584f:	4b                   	dec    %bx
    5850:	4b                   	dec    %bx
    5851:	4b                   	dec    %bx
    5852:	4b                   	dec    %bx
    5853:	4b                   	dec    %bx
    5854:	4b                   	dec    %bx
    5855:	4b                   	dec    %bx
    5856:	4b                   	dec    %bx
    5857:	4b                   	dec    %bx
    5858:	4b                   	dec    %bx
    5859:	4b                   	dec    %bx
    585a:	4b                   	dec    %bx
    585b:	4b                   	dec    %bx
    585c:	4b                   	dec    %bx
    585d:	4b                   	dec    %bx
    585e:	4b                   	dec    %bx
    585f:	4b                   	dec    %bx
    5860:	4b                   	dec    %bx
    5861:	4b                   	dec    %bx
    5862:	4b                   	dec    %bx
    5863:	4b                   	dec    %bx
    5864:	4b                   	dec    %bx
    5865:	4b                   	dec    %bx
    5866:	4b                   	dec    %bx
    5867:	4b                   	dec    %bx
    5868:	4b                   	dec    %bx
    5869:	4b                   	dec    %bx
    586a:	4b                   	dec    %bx
    586b:	4b                   	dec    %bx
    586c:	4b                   	dec    %bx
    586d:	4b                   	dec    %bx
    586e:	4b                   	dec    %bx
    586f:	4b                   	dec    %bx
    5870:	4b                   	dec    %bx
    5871:	4b                   	dec    %bx
    5872:	4b                   	dec    %bx
    5873:	4b                   	dec    %bx
    5874:	4b                   	dec    %bx
    5875:	4b                   	dec    %bx
    5876:	4b                   	dec    %bx
    5877:	4b                   	dec    %bx
    5878:	4b                   	dec    %bx
    5879:	4b                   	dec    %bx
    587a:	4b                   	dec    %bx
    587b:	4b                   	dec    %bx
    587c:	4b                   	dec    %bx
    587d:	4b                   	dec    %bx
    587e:	4b                   	dec    %bx
    587f:	4b                   	dec    %bx
    5880:	4b                   	dec    %bx
    5881:	4b                   	dec    %bx
    5882:	4b                   	dec    %bx
    5883:	4b                   	dec    %bx
    5884:	4b                   	dec    %bx
    5885:	4b                   	dec    %bx
    5886:	4b                   	dec    %bx
    5887:	4b                   	dec    %bx
    5888:	4b                   	dec    %bx
    5889:	4b                   	dec    %bx
    588a:	4b                   	dec    %bx
    588b:	4b                   	dec    %bx
    588c:	4b                   	dec    %bx
    588d:	4b                   	dec    %bx
    588e:	4b                   	dec    %bx
    588f:	4b                   	dec    %bx
    5890:	4b                   	dec    %bx
    5891:	4b                   	dec    %bx
    5892:	4b                   	dec    %bx
    5893:	4b                   	dec    %bx
    5894:	4b                   	dec    %bx
    5895:	4b                   	dec    %bx
    5896:	4b                   	dec    %bx
    5897:	4b                   	dec    %bx
    5898:	4b                   	dec    %bx
    5899:	4b                   	dec    %bx
    589a:	4b                   	dec    %bx
    589b:	4b                   	dec    %bx
    589c:	4b                   	dec    %bx
    589d:	4b                   	dec    %bx
    589e:	4b                   	dec    %bx
    589f:	4b                   	dec    %bx
    58a0:	4b                   	dec    %bx
    58a1:	4b                   	dec    %bx
    58a2:	4b                   	dec    %bx
    58a3:	4b                   	dec    %bx
    58a4:	4b                   	dec    %bx
    58a5:	4b                   	dec    %bx
    58a6:	4b                   	dec    %bx
    58a7:	4b                   	dec    %bx
    58a8:	4b                   	dec    %bx
    58a9:	4b                   	dec    %bx
    58aa:	4b                   	dec    %bx
    58ab:	4b                   	dec    %bx
    58ac:	4b                   	dec    %bx
    58ad:	4b                   	dec    %bx
    58ae:	4b                   	dec    %bx
    58af:	4b                   	dec    %bx
    58b0:	4b                   	dec    %bx
    58b1:	4b                   	dec    %bx
    58b2:	4b                   	dec    %bx
    58b3:	4b                   	dec    %bx
    58b4:	4b                   	dec    %bx
    58b5:	4b                   	dec    %bx
    58b6:	4b                   	dec    %bx
    58b7:	4b                   	dec    %bx
    58b8:	4b                   	dec    %bx
    58b9:	4b                   	dec    %bx
    58ba:	4b                   	dec    %bx
    58bb:	4b                   	dec    %bx
    58bc:	4b                   	dec    %bx
    58bd:	4b                   	dec    %bx
    58be:	4b                   	dec    %bx
    58bf:	4b                   	dec    %bx
    58c0:	4b                   	dec    %bx
    58c1:	4b                   	dec    %bx
    58c2:	4b                   	dec    %bx
    58c3:	4b                   	dec    %bx
    58c4:	4b                   	dec    %bx
    58c5:	4b                   	dec    %bx
    58c6:	4b                   	dec    %bx
    58c7:	4b                   	dec    %bx
    58c8:	4b                   	dec    %bx
    58c9:	4b                   	dec    %bx
    58ca:	4b                   	dec    %bx
    58cb:	4b                   	dec    %bx
    58cc:	4b                   	dec    %bx
    58cd:	4b                   	dec    %bx
    58ce:	4b                   	dec    %bx
    58cf:	4b                   	dec    %bx
    58d0:	4b                   	dec    %bx
    58d1:	4b                   	dec    %bx
    58d2:	4b                   	dec    %bx
    58d3:	4b                   	dec    %bx
    58d4:	4b                   	dec    %bx
    58d5:	4b                   	dec    %bx
    58d6:	4b                   	dec    %bx
    58d7:	4b                   	dec    %bx
    58d8:	4b                   	dec    %bx
    58d9:	4b                   	dec    %bx
    58da:	4b                   	dec    %bx
    58db:	4b                   	dec    %bx
    58dc:	4b                   	dec    %bx
    58dd:	4b                   	dec    %bx
    58de:	4b                   	dec    %bx
    58df:	4b                   	dec    %bx
    58e0:	4b                   	dec    %bx
    58e1:	4b                   	dec    %bx
    58e2:	4b                   	dec    %bx
    58e3:	4b                   	dec    %bx
    58e4:	4b                   	dec    %bx
    58e5:	4b                   	dec    %bx
    58e6:	4b                   	dec    %bx
    58e7:	4b                   	dec    %bx
    58e8:	4b                   	dec    %bx
    58e9:	4b                   	dec    %bx
    58ea:	4b                   	dec    %bx
    58eb:	4b                   	dec    %bx
    58ec:	4b                   	dec    %bx
    58ed:	4b                   	dec    %bx
    58ee:	4b                   	dec    %bx
    58ef:	4b                   	dec    %bx
    58f0:	4b                   	dec    %bx
    58f1:	4b                   	dec    %bx
    58f2:	4b                   	dec    %bx
    58f3:	4b                   	dec    %bx
    58f4:	4b                   	dec    %bx
    58f5:	4b                   	dec    %bx
    58f6:	4b                   	dec    %bx
    58f7:	4b                   	dec    %bx
    58f8:	4b                   	dec    %bx
    58f9:	4b                   	dec    %bx
    58fa:	4b                   	dec    %bx
    58fb:	4b                   	dec    %bx
    58fc:	4b                   	dec    %bx
    58fd:	4b                   	dec    %bx
    58fe:	4b                   	dec    %bx
    58ff:	4b                   	dec    %bx
    5900:	4b                   	dec    %bx
    5901:	4b                   	dec    %bx
    5902:	4b                   	dec    %bx
    5903:	4b                   	dec    %bx
    5904:	4b                   	dec    %bx
    5905:	4b                   	dec    %bx
    5906:	4b                   	dec    %bx
    5907:	4b                   	dec    %bx
    5908:	4b                   	dec    %bx
    5909:	4b                   	dec    %bx
    590a:	4b                   	dec    %bx
    590b:	4b                   	dec    %bx
    590c:	4b                   	dec    %bx
    590d:	4b                   	dec    %bx
    590e:	4b                   	dec    %bx
    590f:	4b                   	dec    %bx
    5910:	4b                   	dec    %bx
    5911:	4b                   	dec    %bx
    5912:	4b                   	dec    %bx
    5913:	4b                   	dec    %bx
    5914:	4b                   	dec    %bx
    5915:	4b                   	dec    %bx
    5916:	4b                   	dec    %bx
    5917:	4b                   	dec    %bx
    5918:	4b                   	dec    %bx
    5919:	4b                   	dec    %bx
    591a:	4b                   	dec    %bx
    591b:	4b                   	dec    %bx
    591c:	4b                   	dec    %bx
    591d:	4b                   	dec    %bx
    591e:	4b                   	dec    %bx
    591f:	4b                   	dec    %bx
    5920:	4b                   	dec    %bx
    5921:	4b                   	dec    %bx
    5922:	4b                   	dec    %bx
    5923:	4b                   	dec    %bx
    5924:	4b                   	dec    %bx
    5925:	4b                   	dec    %bx
    5926:	4b                   	dec    %bx
    5927:	4b                   	dec    %bx
    5928:	4b                   	dec    %bx
    5929:	4b                   	dec    %bx
    592a:	4b                   	dec    %bx
    592b:	4b                   	dec    %bx
    592c:	4b                   	dec    %bx
    592d:	4b                   	dec    %bx
    592e:	4b                   	dec    %bx
    592f:	4b                   	dec    %bx
    5930:	4b                   	dec    %bx
    5931:	4b                   	dec    %bx
    5932:	4b                   	dec    %bx
    5933:	4b                   	dec    %bx
    5934:	4b                   	dec    %bx
    5935:	4b                   	dec    %bx
    5936:	4b                   	dec    %bx
    5937:	4b                   	dec    %bx
    5938:	4b                   	dec    %bx
    5939:	4b                   	dec    %bx
    593a:	4b                   	dec    %bx
    593b:	4b                   	dec    %bx
    593c:	4b                   	dec    %bx
    593d:	4b                   	dec    %bx
    593e:	4b                   	dec    %bx
    593f:	4b                   	dec    %bx
    5940:	4b                   	dec    %bx
    5941:	4b                   	dec    %bx
    5942:	4b                   	dec    %bx
    5943:	4b                   	dec    %bx
    5944:	4b                   	dec    %bx
    5945:	4b                   	dec    %bx
    5946:	4b                   	dec    %bx
    5947:	4b                   	dec    %bx
    5948:	4b                   	dec    %bx
    5949:	4b                   	dec    %bx
    594a:	4b                   	dec    %bx
    594b:	4b                   	dec    %bx
    594c:	4b                   	dec    %bx
    594d:	4b                   	dec    %bx
    594e:	4b                   	dec    %bx
    594f:	4b                   	dec    %bx
    5950:	4b                   	dec    %bx
    5951:	4b                   	dec    %bx
    5952:	4b                   	dec    %bx
    5953:	4b                   	dec    %bx
    5954:	4b                   	dec    %bx
    5955:	4b                   	dec    %bx
    5956:	4b                   	dec    %bx
    5957:	4b                   	dec    %bx
    5958:	4b                   	dec    %bx
    5959:	4b                   	dec    %bx
    595a:	4b                   	dec    %bx
    595b:	4b                   	dec    %bx
    595c:	4b                   	dec    %bx
    595d:	4b                   	dec    %bx
    595e:	4b                   	dec    %bx
    595f:	4b                   	dec    %bx
    5960:	4b                   	dec    %bx
    5961:	4b                   	dec    %bx
    5962:	4b                   	dec    %bx
    5963:	4b                   	dec    %bx
    5964:	4b                   	dec    %bx
    5965:	4b                   	dec    %bx
    5966:	4b                   	dec    %bx
    5967:	4b                   	dec    %bx
    5968:	4b                   	dec    %bx
    5969:	4b                   	dec    %bx
    596a:	4b                   	dec    %bx
    596b:	4b                   	dec    %bx
    596c:	4b                   	dec    %bx
    596d:	4b                   	dec    %bx
    596e:	4b                   	dec    %bx
    596f:	4b                   	dec    %bx
    5970:	4b                   	dec    %bx
    5971:	4b                   	dec    %bx
    5972:	4b                   	dec    %bx
    5973:	4b                   	dec    %bx
    5974:	4b                   	dec    %bx
    5975:	4b                   	dec    %bx
    5976:	4b                   	dec    %bx
    5977:	4b                   	dec    %bx
    5978:	4b                   	dec    %bx
    5979:	4b                   	dec    %bx
    597a:	4b                   	dec    %bx
    597b:	4b                   	dec    %bx
    597c:	4b                   	dec    %bx
    597d:	4b                   	dec    %bx
    597e:	4b                   	dec    %bx
    597f:	4b                   	dec    %bx
    5980:	4b                   	dec    %bx
    5981:	4b                   	dec    %bx
    5982:	4b                   	dec    %bx
    5983:	4b                   	dec    %bx
    5984:	4b                   	dec    %bx
    5985:	4b                   	dec    %bx
    5986:	4b                   	dec    %bx
    5987:	4b                   	dec    %bx
    5988:	4b                   	dec    %bx
    5989:	4b                   	dec    %bx
    598a:	4b                   	dec    %bx
    598b:	4b                   	dec    %bx
    598c:	4b                   	dec    %bx
    598d:	4b                   	dec    %bx
    598e:	4b                   	dec    %bx
    598f:	4b                   	dec    %bx
    5990:	4b                   	dec    %bx
    5991:	4b                   	dec    %bx
    5992:	4b                   	dec    %bx
    5993:	4b                   	dec    %bx
    5994:	4b                   	dec    %bx
    5995:	4b                   	dec    %bx
    5996:	4b                   	dec    %bx
    5997:	4b                   	dec    %bx
    5998:	4b                   	dec    %bx
    5999:	4b                   	dec    %bx
    599a:	4b                   	dec    %bx
    599b:	4b                   	dec    %bx
    599c:	4b                   	dec    %bx
    599d:	4b                   	dec    %bx
    599e:	4b                   	dec    %bx
    599f:	4b                   	dec    %bx
    59a0:	4b                   	dec    %bx
    59a1:	4b                   	dec    %bx
    59a2:	4b                   	dec    %bx
    59a3:	4b                   	dec    %bx
    59a4:	4b                   	dec    %bx
    59a5:	4b                   	dec    %bx
    59a6:	4b                   	dec    %bx
    59a7:	4b                   	dec    %bx
    59a8:	4b                   	dec    %bx
    59a9:	4b                   	dec    %bx
    59aa:	4b                   	dec    %bx
    59ab:	4b                   	dec    %bx
    59ac:	4b                   	dec    %bx
    59ad:	4b                   	dec    %bx
    59ae:	4b                   	dec    %bx
    59af:	4b                   	dec    %bx
    59b0:	4b                   	dec    %bx
    59b1:	4b                   	dec    %bx
    59b2:	4b                   	dec    %bx
    59b3:	4b                   	dec    %bx
    59b4:	4b                   	dec    %bx
    59b5:	4b                   	dec    %bx
    59b6:	4b                   	dec    %bx
    59b7:	4b                   	dec    %bx
    59b8:	4b                   	dec    %bx
    59b9:	4b                   	dec    %bx
    59ba:	4b                   	dec    %bx
    59bb:	4b                   	dec    %bx
    59bc:	4b                   	dec    %bx
    59bd:	4b                   	dec    %bx
    59be:	4b                   	dec    %bx
    59bf:	4b                   	dec    %bx
    59c0:	4b                   	dec    %bx
    59c1:	4b                   	dec    %bx
    59c2:	4b                   	dec    %bx
    59c3:	4b                   	dec    %bx
    59c4:	4b                   	dec    %bx
    59c5:	4b                   	dec    %bx
    59c6:	4b                   	dec    %bx
    59c7:	4b                   	dec    %bx
    59c8:	4b                   	dec    %bx
    59c9:	4b                   	dec    %bx
    59ca:	4b                   	dec    %bx
    59cb:	4b                   	dec    %bx
    59cc:	4b                   	dec    %bx
    59cd:	4b                   	dec    %bx
    59ce:	4b                   	dec    %bx
    59cf:	4b                   	dec    %bx
    59d0:	4b                   	dec    %bx
    59d1:	4b                   	dec    %bx
    59d2:	4b                   	dec    %bx
    59d3:	4b                   	dec    %bx
    59d4:	4b                   	dec    %bx
    59d5:	4b                   	dec    %bx
    59d6:	4b                   	dec    %bx
    59d7:	4b                   	dec    %bx
    59d8:	4b                   	dec    %bx
    59d9:	4b                   	dec    %bx
    59da:	4b                   	dec    %bx
    59db:	4b                   	dec    %bx
    59dc:	4b                   	dec    %bx
    59dd:	4b                   	dec    %bx
    59de:	4b                   	dec    %bx
    59df:	4b                   	dec    %bx
    59e0:	4b                   	dec    %bx
    59e1:	4b                   	dec    %bx
    59e2:	4b                   	dec    %bx
    59e3:	4b                   	dec    %bx
    59e4:	4b                   	dec    %bx
    59e5:	4b                   	dec    %bx
    59e6:	4b                   	dec    %bx
    59e7:	4b                   	dec    %bx
    59e8:	4b                   	dec    %bx
    59e9:	4b                   	dec    %bx
    59ea:	4b                   	dec    %bx
    59eb:	4b                   	dec    %bx
    59ec:	4b                   	dec    %bx
    59ed:	4b                   	dec    %bx
    59ee:	4b                   	dec    %bx
    59ef:	4b                   	dec    %bx
    59f0:	4b                   	dec    %bx
    59f1:	4b                   	dec    %bx
    59f2:	4b                   	dec    %bx
    59f3:	4b                   	dec    %bx
    59f4:	4b                   	dec    %bx
    59f5:	4b                   	dec    %bx
    59f6:	4b                   	dec    %bx
    59f7:	4b                   	dec    %bx
    59f8:	4b                   	dec    %bx
    59f9:	4b                   	dec    %bx
    59fa:	4b                   	dec    %bx
    59fb:	4b                   	dec    %bx
    59fc:	4b                   	dec    %bx
    59fd:	4b                   	dec    %bx
    59fe:	4b                   	dec    %bx
    59ff:	4b                   	dec    %bx
    5a00:	4b                   	dec    %bx
    5a01:	4b                   	dec    %bx
    5a02:	4b                   	dec    %bx
    5a03:	4b                   	dec    %bx
    5a04:	4b                   	dec    %bx
    5a05:	4b                   	dec    %bx
    5a06:	4b                   	dec    %bx
    5a07:	4b                   	dec    %bx
    5a08:	4b                   	dec    %bx
    5a09:	4b                   	dec    %bx
    5a0a:	4b                   	dec    %bx
    5a0b:	4b                   	dec    %bx
    5a0c:	4b                   	dec    %bx
    5a0d:	4b                   	dec    %bx
    5a0e:	4b                   	dec    %bx
    5a0f:	4b                   	dec    %bx
    5a10:	4b                   	dec    %bx
    5a11:	4b                   	dec    %bx
    5a12:	4b                   	dec    %bx
    5a13:	4b                   	dec    %bx
    5a14:	4b                   	dec    %bx
    5a15:	4b                   	dec    %bx
    5a16:	4b                   	dec    %bx
    5a17:	4b                   	dec    %bx
    5a18:	4b                   	dec    %bx
    5a19:	4b                   	dec    %bx
    5a1a:	4b                   	dec    %bx
    5a1b:	4b                   	dec    %bx
    5a1c:	4b                   	dec    %bx
    5a1d:	4b                   	dec    %bx
    5a1e:	4b                   	dec    %bx
    5a1f:	4b                   	dec    %bx
    5a20:	4b                   	dec    %bx
    5a21:	4b                   	dec    %bx
    5a22:	4b                   	dec    %bx
    5a23:	4b                   	dec    %bx
    5a24:	4b                   	dec    %bx
    5a25:	4b                   	dec    %bx
    5a26:	4b                   	dec    %bx
    5a27:	4b                   	dec    %bx
    5a28:	4b                   	dec    %bx
    5a29:	4b                   	dec    %bx
    5a2a:	4b                   	dec    %bx
    5a2b:	4b                   	dec    %bx
    5a2c:	4b                   	dec    %bx
    5a2d:	4b                   	dec    %bx
    5a2e:	4b                   	dec    %bx
    5a2f:	4b                   	dec    %bx
    5a30:	4b                   	dec    %bx
    5a31:	4b                   	dec    %bx
    5a32:	4b                   	dec    %bx
    5a33:	4b                   	dec    %bx
    5a34:	4b                   	dec    %bx
    5a35:	4b                   	dec    %bx
    5a36:	4b                   	dec    %bx
    5a37:	4b                   	dec    %bx
    5a38:	4b                   	dec    %bx
    5a39:	4b                   	dec    %bx
    5a3a:	4b                   	dec    %bx
    5a3b:	4b                   	dec    %bx
    5a3c:	4b                   	dec    %bx
    5a3d:	4b                   	dec    %bx
    5a3e:	4b                   	dec    %bx
    5a3f:	4b                   	dec    %bx
    5a40:	4b                   	dec    %bx
    5a41:	4b                   	dec    %bx
    5a42:	4b                   	dec    %bx
    5a43:	4b                   	dec    %bx
    5a44:	4b                   	dec    %bx
    5a45:	4b                   	dec    %bx
    5a46:	4b                   	dec    %bx
    5a47:	4b                   	dec    %bx
    5a48:	4b                   	dec    %bx
    5a49:	4b                   	dec    %bx
    5a4a:	4b                   	dec    %bx
    5a4b:	4b                   	dec    %bx
    5a4c:	4b                   	dec    %bx
    5a4d:	4b                   	dec    %bx
    5a4e:	4b                   	dec    %bx
    5a4f:	4b                   	dec    %bx
    5a50:	4b                   	dec    %bx
    5a51:	4b                   	dec    %bx
    5a52:	4b                   	dec    %bx
    5a53:	4b                   	dec    %bx
    5a54:	4b                   	dec    %bx
    5a55:	4b                   	dec    %bx
    5a56:	4b                   	dec    %bx
    5a57:	4b                   	dec    %bx
    5a58:	4b                   	dec    %bx
    5a59:	4b                   	dec    %bx
    5a5a:	4b                   	dec    %bx
    5a5b:	4b                   	dec    %bx
    5a5c:	4b                   	dec    %bx
    5a5d:	4b                   	dec    %bx
    5a5e:	4b                   	dec    %bx
    5a5f:	4b                   	dec    %bx
    5a60:	4b                   	dec    %bx
    5a61:	4b                   	dec    %bx
    5a62:	4b                   	dec    %bx
    5a63:	4b                   	dec    %bx
    5a64:	4b                   	dec    %bx
    5a65:	4b                   	dec    %bx
    5a66:	4b                   	dec    %bx
    5a67:	4b                   	dec    %bx
    5a68:	4b                   	dec    %bx
    5a69:	4b                   	dec    %bx
    5a6a:	4b                   	dec    %bx
    5a6b:	4b                   	dec    %bx
    5a6c:	4b                   	dec    %bx
    5a6d:	4b                   	dec    %bx
    5a6e:	4b                   	dec    %bx
    5a6f:	4b                   	dec    %bx
    5a70:	4b                   	dec    %bx
    5a71:	4b                   	dec    %bx
    5a72:	4b                   	dec    %bx
    5a73:	4b                   	dec    %bx
    5a74:	4b                   	dec    %bx
    5a75:	4b                   	dec    %bx
    5a76:	4b                   	dec    %bx
    5a77:	4b                   	dec    %bx
    5a78:	4b                   	dec    %bx
    5a79:	4b                   	dec    %bx
    5a7a:	4b                   	dec    %bx
    5a7b:	4b                   	dec    %bx
    5a7c:	4b                   	dec    %bx
    5a7d:	4b                   	dec    %bx
    5a7e:	4b                   	dec    %bx
    5a7f:	4b                   	dec    %bx
    5a80:	4b                   	dec    %bx
    5a81:	4b                   	dec    %bx
    5a82:	4b                   	dec    %bx
    5a83:	4b                   	dec    %bx
    5a84:	4b                   	dec    %bx
    5a85:	4b                   	dec    %bx
    5a86:	4b                   	dec    %bx
    5a87:	4b                   	dec    %bx
    5a88:	4b                   	dec    %bx
    5a89:	4b                   	dec    %bx
    5a8a:	4b                   	dec    %bx
    5a8b:	4b                   	dec    %bx
    5a8c:	4b                   	dec    %bx
    5a8d:	4b                   	dec    %bx
    5a8e:	4b                   	dec    %bx
    5a8f:	4b                   	dec    %bx
    5a90:	4b                   	dec    %bx
    5a91:	4b                   	dec    %bx
    5a92:	4b                   	dec    %bx
    5a93:	4b                   	dec    %bx
    5a94:	4b                   	dec    %bx
    5a95:	4b                   	dec    %bx
    5a96:	4b                   	dec    %bx
    5a97:	4b                   	dec    %bx
    5a98:	4b                   	dec    %bx
    5a99:	4b                   	dec    %bx
    5a9a:	4b                   	dec    %bx
    5a9b:	4b                   	dec    %bx
    5a9c:	4b                   	dec    %bx
    5a9d:	4b                   	dec    %bx
    5a9e:	4b                   	dec    %bx
    5a9f:	4b                   	dec    %bx
    5aa0:	4b                   	dec    %bx
    5aa1:	4b                   	dec    %bx
    5aa2:	4b                   	dec    %bx
    5aa3:	4b                   	dec    %bx
    5aa4:	4b                   	dec    %bx
    5aa5:	4b                   	dec    %bx
    5aa6:	4b                   	dec    %bx
    5aa7:	4b                   	dec    %bx
    5aa8:	4b                   	dec    %bx
    5aa9:	4b                   	dec    %bx
    5aaa:	4b                   	dec    %bx
    5aab:	4b                   	dec    %bx
    5aac:	4b                   	dec    %bx
    5aad:	4b                   	dec    %bx
    5aae:	4b                   	dec    %bx
    5aaf:	4b                   	dec    %bx
    5ab0:	4b                   	dec    %bx
    5ab1:	4b                   	dec    %bx
    5ab2:	4b                   	dec    %bx
    5ab3:	4b                   	dec    %bx
    5ab4:	4b                   	dec    %bx
    5ab5:	4b                   	dec    %bx
    5ab6:	4b                   	dec    %bx
    5ab7:	4b                   	dec    %bx
    5ab8:	4b                   	dec    %bx
    5ab9:	4b                   	dec    %bx
    5aba:	4b                   	dec    %bx
    5abb:	4b                   	dec    %bx
    5abc:	4b                   	dec    %bx
    5abd:	4b                   	dec    %bx
    5abe:	4b                   	dec    %bx
    5abf:	4b                   	dec    %bx
    5ac0:	4b                   	dec    %bx
    5ac1:	4b                   	dec    %bx
    5ac2:	4b                   	dec    %bx
    5ac3:	4b                   	dec    %bx
    5ac4:	4b                   	dec    %bx
    5ac5:	4b                   	dec    %bx
    5ac6:	4b                   	dec    %bx
    5ac7:	4b                   	dec    %bx
    5ac8:	4b                   	dec    %bx
    5ac9:	4b                   	dec    %bx
    5aca:	4b                   	dec    %bx
    5acb:	4b                   	dec    %bx
    5acc:	4b                   	dec    %bx
    5acd:	4b                   	dec    %bx
    5ace:	4b                   	dec    %bx
    5acf:	4b                   	dec    %bx
    5ad0:	4b                   	dec    %bx
    5ad1:	4b                   	dec    %bx
    5ad2:	4b                   	dec    %bx
    5ad3:	4b                   	dec    %bx
    5ad4:	4b                   	dec    %bx
    5ad5:	4b                   	dec    %bx
    5ad6:	4b                   	dec    %bx
    5ad7:	4b                   	dec    %bx
    5ad8:	4b                   	dec    %bx
    5ad9:	4b                   	dec    %bx
    5ada:	4b                   	dec    %bx
    5adb:	4b                   	dec    %bx
    5adc:	4b                   	dec    %bx
    5add:	4b                   	dec    %bx
    5ade:	4b                   	dec    %bx
    5adf:	4b                   	dec    %bx
    5ae0:	4b                   	dec    %bx
    5ae1:	4b                   	dec    %bx
    5ae2:	4b                   	dec    %bx
    5ae3:	4b                   	dec    %bx
    5ae4:	4b                   	dec    %bx
    5ae5:	4b                   	dec    %bx
    5ae6:	4b                   	dec    %bx
    5ae7:	4b                   	dec    %bx
    5ae8:	4b                   	dec    %bx
    5ae9:	4b                   	dec    %bx
    5aea:	4b                   	dec    %bx
    5aeb:	4b                   	dec    %bx
    5aec:	4b                   	dec    %bx
    5aed:	4b                   	dec    %bx
    5aee:	4b                   	dec    %bx
    5aef:	4b                   	dec    %bx
    5af0:	4b                   	dec    %bx
    5af1:	4b                   	dec    %bx
    5af2:	4b                   	dec    %bx
    5af3:	4b                   	dec    %bx
    5af4:	4b                   	dec    %bx
    5af5:	4b                   	dec    %bx
    5af6:	4b                   	dec    %bx
    5af7:	4b                   	dec    %bx
    5af8:	4b                   	dec    %bx
    5af9:	4b                   	dec    %bx
    5afa:	4b                   	dec    %bx
    5afb:	4b                   	dec    %bx
    5afc:	4b                   	dec    %bx
    5afd:	4b                   	dec    %bx
    5afe:	4b                   	dec    %bx
    5aff:	4b                   	dec    %bx
    5b00:	4b                   	dec    %bx
    5b01:	4b                   	dec    %bx
    5b02:	4b                   	dec    %bx
    5b03:	4b                   	dec    %bx
    5b04:	4b                   	dec    %bx
    5b05:	4b                   	dec    %bx
    5b06:	4b                   	dec    %bx
    5b07:	4b                   	dec    %bx
    5b08:	4b                   	dec    %bx
    5b09:	4b                   	dec    %bx
    5b0a:	4b                   	dec    %bx
    5b0b:	4b                   	dec    %bx
    5b0c:	4b                   	dec    %bx
    5b0d:	4b                   	dec    %bx
    5b0e:	4b                   	dec    %bx
    5b0f:	4b                   	dec    %bx
    5b10:	4b                   	dec    %bx
    5b11:	4b                   	dec    %bx
    5b12:	4b                   	dec    %bx
    5b13:	4b                   	dec    %bx
    5b14:	4b                   	dec    %bx
    5b15:	4b                   	dec    %bx
    5b16:	4b                   	dec    %bx
    5b17:	4b                   	dec    %bx
    5b18:	4b                   	dec    %bx
    5b19:	4b                   	dec    %bx
    5b1a:	4b                   	dec    %bx
    5b1b:	4b                   	dec    %bx
    5b1c:	4b                   	dec    %bx
    5b1d:	4b                   	dec    %bx
    5b1e:	4b                   	dec    %bx
    5b1f:	4b                   	dec    %bx
    5b20:	4b                   	dec    %bx
    5b21:	4b                   	dec    %bx
    5b22:	4b                   	dec    %bx
    5b23:	4b                   	dec    %bx
    5b24:	4b                   	dec    %bx
    5b25:	4b                   	dec    %bx
    5b26:	4b                   	dec    %bx
    5b27:	4b                   	dec    %bx
    5b28:	4b                   	dec    %bx
    5b29:	4b                   	dec    %bx
    5b2a:	4b                   	dec    %bx
    5b2b:	4b                   	dec    %bx
    5b2c:	4b                   	dec    %bx
    5b2d:	4b                   	dec    %bx
    5b2e:	4b                   	dec    %bx
    5b2f:	4b                   	dec    %bx
    5b30:	4b                   	dec    %bx
    5b31:	4b                   	dec    %bx
    5b32:	4b                   	dec    %bx
    5b33:	4b                   	dec    %bx
    5b34:	4b                   	dec    %bx
    5b35:	4b                   	dec    %bx
    5b36:	4b                   	dec    %bx
    5b37:	4b                   	dec    %bx
    5b38:	4b                   	dec    %bx
    5b39:	4b                   	dec    %bx
    5b3a:	4b                   	dec    %bx
    5b3b:	4b                   	dec    %bx
    5b3c:	4b                   	dec    %bx
    5b3d:	4b                   	dec    %bx
    5b3e:	4b                   	dec    %bx
    5b3f:	4b                   	dec    %bx
    5b40:	4b                   	dec    %bx
    5b41:	4b                   	dec    %bx
    5b42:	4b                   	dec    %bx
    5b43:	4b                   	dec    %bx
    5b44:	4b                   	dec    %bx
    5b45:	4b                   	dec    %bx
    5b46:	4b                   	dec    %bx
    5b47:	4b                   	dec    %bx
    5b48:	4b                   	dec    %bx
    5b49:	4b                   	dec    %bx
    5b4a:	4b                   	dec    %bx
    5b4b:	4b                   	dec    %bx
    5b4c:	4b                   	dec    %bx
    5b4d:	4b                   	dec    %bx
    5b4e:	4b                   	dec    %bx
    5b4f:	4b                   	dec    %bx
    5b50:	4b                   	dec    %bx
    5b51:	4b                   	dec    %bx
    5b52:	4b                   	dec    %bx
    5b53:	4b                   	dec    %bx
    5b54:	4b                   	dec    %bx
    5b55:	4b                   	dec    %bx
    5b56:	4b                   	dec    %bx
    5b57:	4b                   	dec    %bx
    5b58:	4b                   	dec    %bx
    5b59:	4b                   	dec    %bx
    5b5a:	4b                   	dec    %bx
    5b5b:	4b                   	dec    %bx
    5b5c:	4b                   	dec    %bx
    5b5d:	4b                   	dec    %bx
    5b5e:	4b                   	dec    %bx
    5b5f:	4b                   	dec    %bx
    5b60:	4b                   	dec    %bx
    5b61:	4b                   	dec    %bx
    5b62:	4b                   	dec    %bx
    5b63:	4b                   	dec    %bx
    5b64:	4b                   	dec    %bx
    5b65:	4b                   	dec    %bx
    5b66:	4b                   	dec    %bx
    5b67:	4b                   	dec    %bx
    5b68:	4b                   	dec    %bx
    5b69:	4b                   	dec    %bx
    5b6a:	4b                   	dec    %bx
    5b6b:	4b                   	dec    %bx
    5b6c:	4b                   	dec    %bx
    5b6d:	4b                   	dec    %bx
    5b6e:	4b                   	dec    %bx
    5b6f:	4b                   	dec    %bx
    5b70:	4b                   	dec    %bx
    5b71:	4b                   	dec    %bx
    5b72:	4b                   	dec    %bx
    5b73:	4b                   	dec    %bx
    5b74:	4b                   	dec    %bx
    5b75:	4b                   	dec    %bx
    5b76:	4b                   	dec    %bx
    5b77:	4b                   	dec    %bx
    5b78:	4b                   	dec    %bx
    5b79:	4b                   	dec    %bx
    5b7a:	4b                   	dec    %bx
    5b7b:	4b                   	dec    %bx
    5b7c:	4b                   	dec    %bx
    5b7d:	4b                   	dec    %bx
    5b7e:	4b                   	dec    %bx
    5b7f:	4b                   	dec    %bx
    5b80:	4b                   	dec    %bx
    5b81:	4b                   	dec    %bx
    5b82:	4b                   	dec    %bx
    5b83:	4b                   	dec    %bx
    5b84:	4b                   	dec    %bx
    5b85:	4b                   	dec    %bx
    5b86:	4b                   	dec    %bx
    5b87:	4b                   	dec    %bx
    5b88:	4b                   	dec    %bx
    5b89:	4b                   	dec    %bx
    5b8a:	4b                   	dec    %bx
    5b8b:	4b                   	dec    %bx
    5b8c:	4b                   	dec    %bx
    5b8d:	4b                   	dec    %bx
    5b8e:	4b                   	dec    %bx
    5b8f:	4b                   	dec    %bx
    5b90:	4b                   	dec    %bx
    5b91:	4b                   	dec    %bx
    5b92:	4b                   	dec    %bx
    5b93:	4b                   	dec    %bx
    5b94:	4b                   	dec    %bx
    5b95:	4b                   	dec    %bx
    5b96:	4b                   	dec    %bx
    5b97:	4b                   	dec    %bx
    5b98:	4b                   	dec    %bx
    5b99:	4b                   	dec    %bx
    5b9a:	4b                   	dec    %bx
    5b9b:	4b                   	dec    %bx
    5b9c:	4b                   	dec    %bx
    5b9d:	4b                   	dec    %bx
    5b9e:	4b                   	dec    %bx
    5b9f:	4b                   	dec    %bx
    5ba0:	4b                   	dec    %bx
    5ba1:	4b                   	dec    %bx
    5ba2:	4b                   	dec    %bx
    5ba3:	4b                   	dec    %bx
    5ba4:	4b                   	dec    %bx
    5ba5:	4b                   	dec    %bx
    5ba6:	4b                   	dec    %bx
    5ba7:	4b                   	dec    %bx
    5ba8:	4b                   	dec    %bx
    5ba9:	4b                   	dec    %bx
    5baa:	4b                   	dec    %bx
    5bab:	4b                   	dec    %bx
    5bac:	4b                   	dec    %bx
    5bad:	4b                   	dec    %bx
    5bae:	4b                   	dec    %bx
    5baf:	4b                   	dec    %bx
    5bb0:	4b                   	dec    %bx
    5bb1:	4b                   	dec    %bx
    5bb2:	4b                   	dec    %bx
    5bb3:	4b                   	dec    %bx
    5bb4:	4b                   	dec    %bx
    5bb5:	4b                   	dec    %bx
    5bb6:	4b                   	dec    %bx
    5bb7:	4b                   	dec    %bx
    5bb8:	4b                   	dec    %bx
    5bb9:	4b                   	dec    %bx
    5bba:	4b                   	dec    %bx
    5bbb:	4b                   	dec    %bx
    5bbc:	4b                   	dec    %bx
    5bbd:	4b                   	dec    %bx
    5bbe:	4b                   	dec    %bx
    5bbf:	4b                   	dec    %bx
    5bc0:	4b                   	dec    %bx
    5bc1:	4b                   	dec    %bx
    5bc2:	4b                   	dec    %bx
    5bc3:	4b                   	dec    %bx
    5bc4:	4b                   	dec    %bx
    5bc5:	4b                   	dec    %bx
    5bc6:	4b                   	dec    %bx
    5bc7:	4b                   	dec    %bx
    5bc8:	4b                   	dec    %bx
    5bc9:	4b                   	dec    %bx
    5bca:	4b                   	dec    %bx
    5bcb:	4b                   	dec    %bx
    5bcc:	4b                   	dec    %bx
    5bcd:	4b                   	dec    %bx
    5bce:	4b                   	dec    %bx
    5bcf:	4b                   	dec    %bx
    5bd0:	4b                   	dec    %bx
    5bd1:	4b                   	dec    %bx
    5bd2:	4b                   	dec    %bx
    5bd3:	4b                   	dec    %bx
    5bd4:	4b                   	dec    %bx
    5bd5:	4b                   	dec    %bx
    5bd6:	4b                   	dec    %bx
    5bd7:	4b                   	dec    %bx
    5bd8:	4b                   	dec    %bx
    5bd9:	4b                   	dec    %bx
    5bda:	4b                   	dec    %bx
    5bdb:	4b                   	dec    %bx
    5bdc:	4b                   	dec    %bx
    5bdd:	4b                   	dec    %bx
    5bde:	4b                   	dec    %bx
    5bdf:	4b                   	dec    %bx
    5be0:	4b                   	dec    %bx
    5be1:	4b                   	dec    %bx
    5be2:	4b                   	dec    %bx
    5be3:	4b                   	dec    %bx
    5be4:	4b                   	dec    %bx
    5be5:	4b                   	dec    %bx
    5be6:	4b                   	dec    %bx
    5be7:	4b                   	dec    %bx
    5be8:	4b                   	dec    %bx
    5be9:	4b                   	dec    %bx
    5bea:	4b                   	dec    %bx
    5beb:	4b                   	dec    %bx
    5bec:	4b                   	dec    %bx
    5bed:	4b                   	dec    %bx
    5bee:	4b                   	dec    %bx
    5bef:	4b                   	dec    %bx
    5bf0:	4b                   	dec    %bx
    5bf1:	4b                   	dec    %bx
    5bf2:	4b                   	dec    %bx
    5bf3:	4b                   	dec    %bx
    5bf4:	4b                   	dec    %bx
    5bf5:	4b                   	dec    %bx
    5bf6:	4b                   	dec    %bx
    5bf7:	4b                   	dec    %bx
    5bf8:	4b                   	dec    %bx
    5bf9:	4b                   	dec    %bx
    5bfa:	4b                   	dec    %bx
    5bfb:	4b                   	dec    %bx
    5bfc:	4b                   	dec    %bx
    5bfd:	4b                   	dec    %bx
    5bfe:	4b                   	dec    %bx
    5bff:	4b                   	dec    %bx
    5c00:	4b                   	dec    %bx
    5c01:	4b                   	dec    %bx
    5c02:	4b                   	dec    %bx
    5c03:	4b                   	dec    %bx
    5c04:	4b                   	dec    %bx
    5c05:	4b                   	dec    %bx
    5c06:	4b                   	dec    %bx
    5c07:	4b                   	dec    %bx
    5c08:	4b                   	dec    %bx
    5c09:	4b                   	dec    %bx
    5c0a:	4b                   	dec    %bx
    5c0b:	4b                   	dec    %bx
    5c0c:	4b                   	dec    %bx
    5c0d:	4b                   	dec    %bx
    5c0e:	4b                   	dec    %bx
    5c0f:	4b                   	dec    %bx
    5c10:	4b                   	dec    %bx
    5c11:	4b                   	dec    %bx
    5c12:	4b                   	dec    %bx
    5c13:	4b                   	dec    %bx
    5c14:	4b                   	dec    %bx
    5c15:	4b                   	dec    %bx
    5c16:	4b                   	dec    %bx
    5c17:	4b                   	dec    %bx
    5c18:	4b                   	dec    %bx
    5c19:	4b                   	dec    %bx
    5c1a:	4b                   	dec    %bx
    5c1b:	4b                   	dec    %bx
    5c1c:	4b                   	dec    %bx
    5c1d:	4b                   	dec    %bx
    5c1e:	4b                   	dec    %bx
    5c1f:	4b                   	dec    %bx
    5c20:	4b                   	dec    %bx
    5c21:	4b                   	dec    %bx
    5c22:	4b                   	dec    %bx
    5c23:	4b                   	dec    %bx
    5c24:	4b                   	dec    %bx
    5c25:	4b                   	dec    %bx
    5c26:	4b                   	dec    %bx
    5c27:	4b                   	dec    %bx
    5c28:	4b                   	dec    %bx
    5c29:	4b                   	dec    %bx
    5c2a:	4b                   	dec    %bx
    5c2b:	4b                   	dec    %bx
    5c2c:	4b                   	dec    %bx
    5c2d:	4b                   	dec    %bx
    5c2e:	4b                   	dec    %bx
    5c2f:	4b                   	dec    %bx
    5c30:	4b                   	dec    %bx
    5c31:	4b                   	dec    %bx
    5c32:	4b                   	dec    %bx
    5c33:	4b                   	dec    %bx
    5c34:	4b                   	dec    %bx
    5c35:	4b                   	dec    %bx
    5c36:	4b                   	dec    %bx
    5c37:	4b                   	dec    %bx
    5c38:	4b                   	dec    %bx
    5c39:	4b                   	dec    %bx
    5c3a:	4b                   	dec    %bx
    5c3b:	4b                   	dec    %bx
    5c3c:	4b                   	dec    %bx
    5c3d:	4b                   	dec    %bx
    5c3e:	4b                   	dec    %bx
    5c3f:	4b                   	dec    %bx
    5c40:	4b                   	dec    %bx
    5c41:	4b                   	dec    %bx
    5c42:	4b                   	dec    %bx
    5c43:	4b                   	dec    %bx
    5c44:	4b                   	dec    %bx
    5c45:	4b                   	dec    %bx
    5c46:	4b                   	dec    %bx
    5c47:	4b                   	dec    %bx
    5c48:	4b                   	dec    %bx
    5c49:	4b                   	dec    %bx
    5c4a:	4b                   	dec    %bx
    5c4b:	4b                   	dec    %bx
    5c4c:	4b                   	dec    %bx
    5c4d:	4b                   	dec    %bx
    5c4e:	4b                   	dec    %bx
    5c4f:	4b                   	dec    %bx
    5c50:	4b                   	dec    %bx
    5c51:	4b                   	dec    %bx
    5c52:	4b                   	dec    %bx
    5c53:	4b                   	dec    %bx
    5c54:	4b                   	dec    %bx
    5c55:	4b                   	dec    %bx
    5c56:	4b                   	dec    %bx
    5c57:	4b                   	dec    %bx
    5c58:	4b                   	dec    %bx
    5c59:	4b                   	dec    %bx
    5c5a:	4b                   	dec    %bx
    5c5b:	4b                   	dec    %bx
    5c5c:	4b                   	dec    %bx
    5c5d:	4b                   	dec    %bx
    5c5e:	4b                   	dec    %bx
    5c5f:	4b                   	dec    %bx
    5c60:	4b                   	dec    %bx
    5c61:	4b                   	dec    %bx
    5c62:	4b                   	dec    %bx
    5c63:	4b                   	dec    %bx
    5c64:	4b                   	dec    %bx
    5c65:	4b                   	dec    %bx
    5c66:	4b                   	dec    %bx
    5c67:	4b                   	dec    %bx
    5c68:	4b                   	dec    %bx
    5c69:	4b                   	dec    %bx
    5c6a:	4b                   	dec    %bx
    5c6b:	4b                   	dec    %bx
    5c6c:	4b                   	dec    %bx
    5c6d:	4b                   	dec    %bx
    5c6e:	4b                   	dec    %bx
    5c6f:	4b                   	dec    %bx
    5c70:	4b                   	dec    %bx
    5c71:	4b                   	dec    %bx
    5c72:	4b                   	dec    %bx
    5c73:	4b                   	dec    %bx
    5c74:	4b                   	dec    %bx
    5c75:	4b                   	dec    %bx
    5c76:	4b                   	dec    %bx
    5c77:	4b                   	dec    %bx
    5c78:	4b                   	dec    %bx
    5c79:	4b                   	dec    %bx
    5c7a:	4b                   	dec    %bx
    5c7b:	4b                   	dec    %bx
    5c7c:	4b                   	dec    %bx
    5c7d:	4b                   	dec    %bx
    5c7e:	4b                   	dec    %bx
    5c7f:	4b                   	dec    %bx
    5c80:	4b                   	dec    %bx
    5c81:	4b                   	dec    %bx
    5c82:	4b                   	dec    %bx
    5c83:	4b                   	dec    %bx
    5c84:	4b                   	dec    %bx
    5c85:	4b                   	dec    %bx
    5c86:	4b                   	dec    %bx
    5c87:	4b                   	dec    %bx
    5c88:	4b                   	dec    %bx
    5c89:	4b                   	dec    %bx
    5c8a:	4b                   	dec    %bx
    5c8b:	4b                   	dec    %bx
    5c8c:	4b                   	dec    %bx
    5c8d:	4b                   	dec    %bx
    5c8e:	4b                   	dec    %bx
    5c8f:	4b                   	dec    %bx
    5c90:	4b                   	dec    %bx
    5c91:	4b                   	dec    %bx
    5c92:	4b                   	dec    %bx
    5c93:	4b                   	dec    %bx
    5c94:	4b                   	dec    %bx
    5c95:	4b                   	dec    %bx
    5c96:	4b                   	dec    %bx
    5c97:	4b                   	dec    %bx
    5c98:	4b                   	dec    %bx
    5c99:	4b                   	dec    %bx
    5c9a:	4b                   	dec    %bx
    5c9b:	4b                   	dec    %bx
    5c9c:	4b                   	dec    %bx
    5c9d:	4b                   	dec    %bx
    5c9e:	4b                   	dec    %bx
    5c9f:	4b                   	dec    %bx
    5ca0:	4b                   	dec    %bx
    5ca1:	4b                   	dec    %bx
    5ca2:	4b                   	dec    %bx
    5ca3:	4b                   	dec    %bx
    5ca4:	4b                   	dec    %bx
    5ca5:	4b                   	dec    %bx
    5ca6:	4b                   	dec    %bx
    5ca7:	4b                   	dec    %bx
    5ca8:	4b                   	dec    %bx
    5ca9:	4b                   	dec    %bx
    5caa:	4b                   	dec    %bx
    5cab:	4b                   	dec    %bx
    5cac:	4b                   	dec    %bx
    5cad:	4b                   	dec    %bx
    5cae:	4b                   	dec    %bx
    5caf:	4b                   	dec    %bx
    5cb0:	4b                   	dec    %bx
    5cb1:	4b                   	dec    %bx
    5cb2:	4b                   	dec    %bx
    5cb3:	4b                   	dec    %bx
    5cb4:	4b                   	dec    %bx
    5cb5:	4b                   	dec    %bx
    5cb6:	4b                   	dec    %bx
    5cb7:	4b                   	dec    %bx
    5cb8:	4b                   	dec    %bx
    5cb9:	4b                   	dec    %bx
    5cba:	4b                   	dec    %bx
    5cbb:	4b                   	dec    %bx
    5cbc:	4b                   	dec    %bx
    5cbd:	4b                   	dec    %bx
    5cbe:	4b                   	dec    %bx
    5cbf:	4b                   	dec    %bx
    5cc0:	4b                   	dec    %bx
    5cc1:	4b                   	dec    %bx
    5cc2:	4b                   	dec    %bx
    5cc3:	4b                   	dec    %bx
    5cc4:	4b                   	dec    %bx
    5cc5:	4b                   	dec    %bx
    5cc6:	4b                   	dec    %bx
    5cc7:	4b                   	dec    %bx
    5cc8:	4b                   	dec    %bx
    5cc9:	4b                   	dec    %bx
    5cca:	4b                   	dec    %bx
    5ccb:	4b                   	dec    %bx
    5ccc:	4b                   	dec    %bx
    5ccd:	4b                   	dec    %bx
    5cce:	4b                   	dec    %bx
    5ccf:	4b                   	dec    %bx
    5cd0:	4b                   	dec    %bx
    5cd1:	4b                   	dec    %bx
    5cd2:	4b                   	dec    %bx
    5cd3:	4b                   	dec    %bx
    5cd4:	4b                   	dec    %bx
    5cd5:	4b                   	dec    %bx
    5cd6:	4b                   	dec    %bx
    5cd7:	4b                   	dec    %bx
    5cd8:	4b                   	dec    %bx
    5cd9:	4b                   	dec    %bx
    5cda:	4b                   	dec    %bx
    5cdb:	4b                   	dec    %bx
    5cdc:	4b                   	dec    %bx
    5cdd:	4b                   	dec    %bx
    5cde:	4b                   	dec    %bx
    5cdf:	4b                   	dec    %bx
    5ce0:	4b                   	dec    %bx
    5ce1:	4b                   	dec    %bx
    5ce2:	4b                   	dec    %bx
    5ce3:	4b                   	dec    %bx
    5ce4:	4b                   	dec    %bx
    5ce5:	4b                   	dec    %bx
    5ce6:	4b                   	dec    %bx
    5ce7:	4b                   	dec    %bx
    5ce8:	4b                   	dec    %bx
    5ce9:	4b                   	dec    %bx
    5cea:	4b                   	dec    %bx
    5ceb:	4b                   	dec    %bx
    5cec:	4b                   	dec    %bx
    5ced:	4b                   	dec    %bx
    5cee:	4b                   	dec    %bx
    5cef:	4b                   	dec    %bx
    5cf0:	4b                   	dec    %bx
    5cf1:	4b                   	dec    %bx
    5cf2:	4b                   	dec    %bx
    5cf3:	4b                   	dec    %bx
    5cf4:	4b                   	dec    %bx
    5cf5:	4b                   	dec    %bx
    5cf6:	4b                   	dec    %bx
    5cf7:	4b                   	dec    %bx
    5cf8:	4b                   	dec    %bx
    5cf9:	4b                   	dec    %bx
    5cfa:	4b                   	dec    %bx
    5cfb:	4b                   	dec    %bx
    5cfc:	4b                   	dec    %bx
    5cfd:	4b                   	dec    %bx
    5cfe:	4b                   	dec    %bx
    5cff:	4b                   	dec    %bx
    5d00:	4b                   	dec    %bx
    5d01:	4b                   	dec    %bx
    5d02:	4b                   	dec    %bx
    5d03:	4b                   	dec    %bx
    5d04:	4b                   	dec    %bx
    5d05:	4b                   	dec    %bx
    5d06:	4b                   	dec    %bx
    5d07:	4b                   	dec    %bx
    5d08:	4b                   	dec    %bx
    5d09:	4b                   	dec    %bx
    5d0a:	4b                   	dec    %bx
    5d0b:	4b                   	dec    %bx
    5d0c:	4b                   	dec    %bx
    5d0d:	4b                   	dec    %bx
    5d0e:	4b                   	dec    %bx
    5d0f:	4b                   	dec    %bx
    5d10:	4b                   	dec    %bx
    5d11:	4b                   	dec    %bx
    5d12:	4b                   	dec    %bx
    5d13:	4b                   	dec    %bx
    5d14:	4b                   	dec    %bx
    5d15:	4b                   	dec    %bx
    5d16:	4b                   	dec    %bx
    5d17:	4b                   	dec    %bx
    5d18:	4b                   	dec    %bx
    5d19:	4b                   	dec    %bx
    5d1a:	4b                   	dec    %bx
    5d1b:	4b                   	dec    %bx
    5d1c:	4b                   	dec    %bx
    5d1d:	4b                   	dec    %bx
    5d1e:	4b                   	dec    %bx
    5d1f:	4b                   	dec    %bx
    5d20:	4b                   	dec    %bx
    5d21:	4b                   	dec    %bx
    5d22:	4b                   	dec    %bx
    5d23:	4b                   	dec    %bx
    5d24:	4b                   	dec    %bx
    5d25:	4b                   	dec    %bx
    5d26:	4b                   	dec    %bx
    5d27:	4b                   	dec    %bx
    5d28:	4b                   	dec    %bx
    5d29:	4b                   	dec    %bx
    5d2a:	4b                   	dec    %bx
    5d2b:	4b                   	dec    %bx
    5d2c:	4b                   	dec    %bx
    5d2d:	4b                   	dec    %bx
    5d2e:	4b                   	dec    %bx
    5d2f:	4b                   	dec    %bx
    5d30:	4b                   	dec    %bx
    5d31:	4b                   	dec    %bx
    5d32:	4b                   	dec    %bx
    5d33:	4b                   	dec    %bx
    5d34:	4b                   	dec    %bx
    5d35:	4b                   	dec    %bx
    5d36:	4b                   	dec    %bx
    5d37:	4b                   	dec    %bx
    5d38:	4b                   	dec    %bx
    5d39:	4b                   	dec    %bx
    5d3a:	4b                   	dec    %bx
    5d3b:	4b                   	dec    %bx
    5d3c:	4b                   	dec    %bx
    5d3d:	4b                   	dec    %bx
    5d3e:	4b                   	dec    %bx
    5d3f:	4b                   	dec    %bx
    5d40:	4b                   	dec    %bx
    5d41:	4b                   	dec    %bx
    5d42:	4b                   	dec    %bx
    5d43:	4b                   	dec    %bx
    5d44:	4b                   	dec    %bx
    5d45:	4b                   	dec    %bx
    5d46:	4b                   	dec    %bx
    5d47:	4b                   	dec    %bx
    5d48:	4b                   	dec    %bx
    5d49:	4b                   	dec    %bx
    5d4a:	4b                   	dec    %bx
    5d4b:	4b                   	dec    %bx
    5d4c:	4b                   	dec    %bx
    5d4d:	4b                   	dec    %bx
    5d4e:	4b                   	dec    %bx
    5d4f:	4b                   	dec    %bx
    5d50:	4b                   	dec    %bx
    5d51:	4b                   	dec    %bx
    5d52:	4b                   	dec    %bx
    5d53:	4b                   	dec    %bx
    5d54:	4b                   	dec    %bx
    5d55:	4b                   	dec    %bx
    5d56:	4b                   	dec    %bx
    5d57:	4b                   	dec    %bx
    5d58:	4b                   	dec    %bx
    5d59:	4b                   	dec    %bx
    5d5a:	4b                   	dec    %bx
    5d5b:	4b                   	dec    %bx
    5d5c:	4b                   	dec    %bx
    5d5d:	4b                   	dec    %bx
    5d5e:	4b                   	dec    %bx
    5d5f:	4b                   	dec    %bx
    5d60:	4b                   	dec    %bx
    5d61:	4b                   	dec    %bx
    5d62:	4b                   	dec    %bx
    5d63:	4b                   	dec    %bx
    5d64:	4b                   	dec    %bx
    5d65:	4b                   	dec    %bx
    5d66:	4b                   	dec    %bx
    5d67:	4b                   	dec    %bx
    5d68:	4b                   	dec    %bx
    5d69:	4b                   	dec    %bx
    5d6a:	4b                   	dec    %bx
    5d6b:	4b                   	dec    %bx
    5d6c:	4b                   	dec    %bx
    5d6d:	4b                   	dec    %bx
    5d6e:	4b                   	dec    %bx
    5d6f:	4b                   	dec    %bx
    5d70:	4b                   	dec    %bx
    5d71:	4b                   	dec    %bx
    5d72:	4b                   	dec    %bx
    5d73:	4b                   	dec    %bx
    5d74:	4b                   	dec    %bx
    5d75:	4b                   	dec    %bx
    5d76:	4b                   	dec    %bx
    5d77:	4b                   	dec    %bx
    5d78:	4b                   	dec    %bx
    5d79:	4b                   	dec    %bx
    5d7a:	4b                   	dec    %bx
    5d7b:	4b                   	dec    %bx
    5d7c:	4b                   	dec    %bx
    5d7d:	4b                   	dec    %bx
    5d7e:	4b                   	dec    %bx
    5d7f:	4b                   	dec    %bx
    5d80:	4b                   	dec    %bx
    5d81:	4b                   	dec    %bx
    5d82:	4b                   	dec    %bx
    5d83:	4b                   	dec    %bx
    5d84:	4b                   	dec    %bx
    5d85:	4b                   	dec    %bx
    5d86:	4b                   	dec    %bx
    5d87:	4b                   	dec    %bx
    5d88:	4b                   	dec    %bx
    5d89:	4b                   	dec    %bx
    5d8a:	4b                   	dec    %bx
    5d8b:	4b                   	dec    %bx
    5d8c:	4b                   	dec    %bx
    5d8d:	4b                   	dec    %bx
    5d8e:	4b                   	dec    %bx
    5d8f:	4b                   	dec    %bx
    5d90:	4b                   	dec    %bx
    5d91:	4b                   	dec    %bx
    5d92:	4b                   	dec    %bx
    5d93:	4b                   	dec    %bx
    5d94:	4b                   	dec    %bx
    5d95:	4b                   	dec    %bx
    5d96:	4b                   	dec    %bx
    5d97:	4b                   	dec    %bx
    5d98:	4b                   	dec    %bx
    5d99:	4b                   	dec    %bx
    5d9a:	4b                   	dec    %bx
    5d9b:	4b                   	dec    %bx
    5d9c:	4b                   	dec    %bx
    5d9d:	4b                   	dec    %bx
    5d9e:	4b                   	dec    %bx
    5d9f:	4b                   	dec    %bx
    5da0:	4b                   	dec    %bx
    5da1:	4b                   	dec    %bx
    5da2:	4b                   	dec    %bx
    5da3:	4b                   	dec    %bx
    5da4:	4b                   	dec    %bx
    5da5:	4b                   	dec    %bx
    5da6:	4b                   	dec    %bx
    5da7:	4b                   	dec    %bx
    5da8:	4b                   	dec    %bx
    5da9:	4b                   	dec    %bx
    5daa:	4b                   	dec    %bx
    5dab:	4b                   	dec    %bx
    5dac:	4b                   	dec    %bx
    5dad:	4b                   	dec    %bx
    5dae:	4b                   	dec    %bx
    5daf:	4b                   	dec    %bx
    5db0:	4b                   	dec    %bx
    5db1:	4b                   	dec    %bx
    5db2:	4b                   	dec    %bx
    5db3:	4b                   	dec    %bx
    5db4:	4b                   	dec    %bx
    5db5:	4b                   	dec    %bx
    5db6:	4b                   	dec    %bx
    5db7:	4b                   	dec    %bx
    5db8:	4b                   	dec    %bx
    5db9:	4b                   	dec    %bx
    5dba:	4b                   	dec    %bx
    5dbb:	4b                   	dec    %bx
    5dbc:	4b                   	dec    %bx
    5dbd:	4b                   	dec    %bx
    5dbe:	4b                   	dec    %bx
    5dbf:	4b                   	dec    %bx
    5dc0:	4b                   	dec    %bx
    5dc1:	4b                   	dec    %bx
    5dc2:	4b                   	dec    %bx
    5dc3:	4b                   	dec    %bx
    5dc4:	4b                   	dec    %bx
    5dc5:	4b                   	dec    %bx
    5dc6:	4b                   	dec    %bx
    5dc7:	4b                   	dec    %bx
    5dc8:	4b                   	dec    %bx
    5dc9:	4b                   	dec    %bx
    5dca:	4b                   	dec    %bx
    5dcb:	4b                   	dec    %bx
    5dcc:	4b                   	dec    %bx
    5dcd:	4b                   	dec    %bx
    5dce:	4b                   	dec    %bx
    5dcf:	4b                   	dec    %bx
    5dd0:	4b                   	dec    %bx
    5dd1:	4b                   	dec    %bx
    5dd2:	4b                   	dec    %bx
    5dd3:	4b                   	dec    %bx
    5dd4:	4b                   	dec    %bx
    5dd5:	4b                   	dec    %bx
    5dd6:	4b                   	dec    %bx
    5dd7:	4b                   	dec    %bx
    5dd8:	4b                   	dec    %bx
    5dd9:	4b                   	dec    %bx
    5dda:	4b                   	dec    %bx
    5ddb:	4b                   	dec    %bx
    5ddc:	4b                   	dec    %bx
    5ddd:	4b                   	dec    %bx
    5dde:	4b                   	dec    %bx
    5ddf:	4b                   	dec    %bx
    5de0:	4b                   	dec    %bx
    5de1:	4b                   	dec    %bx
    5de2:	4b                   	dec    %bx
    5de3:	4b                   	dec    %bx
    5de4:	4b                   	dec    %bx
    5de5:	4b                   	dec    %bx
    5de6:	4b                   	dec    %bx
    5de7:	4b                   	dec    %bx
    5de8:	4b                   	dec    %bx
    5de9:	4b                   	dec    %bx
    5dea:	4b                   	dec    %bx
    5deb:	4b                   	dec    %bx
    5dec:	4b                   	dec    %bx
    5ded:	4b                   	dec    %bx
    5dee:	4b                   	dec    %bx
    5def:	4b                   	dec    %bx
    5df0:	4b                   	dec    %bx
    5df1:	4b                   	dec    %bx
    5df2:	4b                   	dec    %bx
    5df3:	4b                   	dec    %bx
    5df4:	4b                   	dec    %bx
    5df5:	4b                   	dec    %bx
    5df6:	4b                   	dec    %bx
    5df7:	4b                   	dec    %bx
    5df8:	4b                   	dec    %bx
    5df9:	4b                   	dec    %bx
    5dfa:	4b                   	dec    %bx
    5dfb:	4b                   	dec    %bx
    5dfc:	4b                   	dec    %bx
    5dfd:	4b                   	dec    %bx
    5dfe:	4b                   	dec    %bx
    5dff:	4b                   	dec    %bx
    5e00:	4b                   	dec    %bx
    5e01:	4b                   	dec    %bx
    5e02:	4b                   	dec    %bx
    5e03:	4b                   	dec    %bx
    5e04:	4b                   	dec    %bx
    5e05:	4b                   	dec    %bx
    5e06:	4b                   	dec    %bx
    5e07:	4b                   	dec    %bx
    5e08:	4b                   	dec    %bx
    5e09:	4b                   	dec    %bx
    5e0a:	4b                   	dec    %bx
    5e0b:	4b                   	dec    %bx
    5e0c:	4b                   	dec    %bx
    5e0d:	4b                   	dec    %bx
    5e0e:	4b                   	dec    %bx
    5e0f:	4b                   	dec    %bx
    5e10:	4b                   	dec    %bx
    5e11:	4b                   	dec    %bx
    5e12:	4b                   	dec    %bx
    5e13:	4b                   	dec    %bx
    5e14:	4b                   	dec    %bx
    5e15:	4b                   	dec    %bx
    5e16:	4b                   	dec    %bx
    5e17:	4b                   	dec    %bx
    5e18:	4b                   	dec    %bx
    5e19:	4b                   	dec    %bx
    5e1a:	4b                   	dec    %bx
    5e1b:	4b                   	dec    %bx
    5e1c:	4b                   	dec    %bx
    5e1d:	4b                   	dec    %bx
    5e1e:	4b                   	dec    %bx
    5e1f:	4b                   	dec    %bx
    5e20:	4b                   	dec    %bx
    5e21:	4b                   	dec    %bx
    5e22:	4b                   	dec    %bx
    5e23:	4b                   	dec    %bx
    5e24:	4b                   	dec    %bx
    5e25:	4b                   	dec    %bx
    5e26:	4b                   	dec    %bx
    5e27:	4b                   	dec    %bx
    5e28:	4b                   	dec    %bx
    5e29:	4b                   	dec    %bx
    5e2a:	4b                   	dec    %bx
    5e2b:	4b                   	dec    %bx
    5e2c:	4b                   	dec    %bx
    5e2d:	4b                   	dec    %bx
    5e2e:	4b                   	dec    %bx
    5e2f:	4b                   	dec    %bx
    5e30:	4b                   	dec    %bx
    5e31:	4b                   	dec    %bx
    5e32:	4b                   	dec    %bx
    5e33:	4b                   	dec    %bx
    5e34:	4b                   	dec    %bx
    5e35:	4b                   	dec    %bx
    5e36:	4b                   	dec    %bx
    5e37:	4b                   	dec    %bx
    5e38:	4b                   	dec    %bx
    5e39:	4b                   	dec    %bx
    5e3a:	4b                   	dec    %bx
    5e3b:	4b                   	dec    %bx
    5e3c:	4b                   	dec    %bx
    5e3d:	4b                   	dec    %bx
    5e3e:	4b                   	dec    %bx
    5e3f:	4b                   	dec    %bx
    5e40:	4b                   	dec    %bx
    5e41:	4b                   	dec    %bx
    5e42:	4b                   	dec    %bx
    5e43:	4b                   	dec    %bx
    5e44:	4b                   	dec    %bx
    5e45:	4b                   	dec    %bx
    5e46:	4b                   	dec    %bx
    5e47:	4b                   	dec    %bx
    5e48:	4b                   	dec    %bx
    5e49:	4b                   	dec    %bx
    5e4a:	4b                   	dec    %bx
    5e4b:	4b                   	dec    %bx
    5e4c:	4b                   	dec    %bx
    5e4d:	4b                   	dec    %bx
    5e4e:	4b                   	dec    %bx
    5e4f:	4b                   	dec    %bx
    5e50:	4b                   	dec    %bx
    5e51:	4b                   	dec    %bx
    5e52:	4b                   	dec    %bx
    5e53:	4b                   	dec    %bx
    5e54:	4b                   	dec    %bx
    5e55:	4b                   	dec    %bx
    5e56:	4b                   	dec    %bx
    5e57:	4b                   	dec    %bx
    5e58:	4b                   	dec    %bx
    5e59:	4b                   	dec    %bx
    5e5a:	4b                   	dec    %bx
    5e5b:	4b                   	dec    %bx
    5e5c:	4b                   	dec    %bx
    5e5d:	4b                   	dec    %bx
    5e5e:	4b                   	dec    %bx
    5e5f:	4b                   	dec    %bx
    5e60:	4b                   	dec    %bx
    5e61:	4b                   	dec    %bx
    5e62:	4b                   	dec    %bx
    5e63:	4b                   	dec    %bx
    5e64:	4b                   	dec    %bx
    5e65:	4b                   	dec    %bx
    5e66:	4b                   	dec    %bx
    5e67:	4b                   	dec    %bx
    5e68:	4b                   	dec    %bx
    5e69:	4b                   	dec    %bx
    5e6a:	4b                   	dec    %bx
    5e6b:	4b                   	dec    %bx
    5e6c:	4b                   	dec    %bx
    5e6d:	4b                   	dec    %bx
    5e6e:	4b                   	dec    %bx
    5e6f:	4b                   	dec    %bx
    5e70:	4b                   	dec    %bx
    5e71:	4b                   	dec    %bx
    5e72:	4b                   	dec    %bx
    5e73:	4b                   	dec    %bx
    5e74:	4b                   	dec    %bx
    5e75:	4b                   	dec    %bx
    5e76:	4b                   	dec    %bx
    5e77:	4b                   	dec    %bx
    5e78:	4b                   	dec    %bx
    5e79:	4b                   	dec    %bx
    5e7a:	4b                   	dec    %bx
    5e7b:	4b                   	dec    %bx
    5e7c:	4b                   	dec    %bx
    5e7d:	4b                   	dec    %bx
    5e7e:	4b                   	dec    %bx
    5e7f:	4b                   	dec    %bx
    5e80:	4b                   	dec    %bx
    5e81:	4b                   	dec    %bx
    5e82:	4b                   	dec    %bx
    5e83:	4b                   	dec    %bx
    5e84:	4b                   	dec    %bx
    5e85:	4b                   	dec    %bx
    5e86:	4b                   	dec    %bx
    5e87:	4b                   	dec    %bx
    5e88:	4b                   	dec    %bx
    5e89:	4b                   	dec    %bx
    5e8a:	4b                   	dec    %bx
    5e8b:	4b                   	dec    %bx
    5e8c:	4b                   	dec    %bx
    5e8d:	4b                   	dec    %bx
    5e8e:	4b                   	dec    %bx
    5e8f:	4b                   	dec    %bx
    5e90:	4b                   	dec    %bx
    5e91:	4b                   	dec    %bx
    5e92:	4b                   	dec    %bx
    5e93:	4b                   	dec    %bx
    5e94:	4b                   	dec    %bx
    5e95:	4b                   	dec    %bx
    5e96:	4b                   	dec    %bx
    5e97:	4b                   	dec    %bx
    5e98:	4b                   	dec    %bx
    5e99:	4b                   	dec    %bx
    5e9a:	4b                   	dec    %bx
    5e9b:	4b                   	dec    %bx
    5e9c:	4b                   	dec    %bx
    5e9d:	4b                   	dec    %bx
    5e9e:	4b                   	dec    %bx
    5e9f:	4b                   	dec    %bx
    5ea0:	4b                   	dec    %bx
    5ea1:	4b                   	dec    %bx
    5ea2:	4b                   	dec    %bx
    5ea3:	4b                   	dec    %bx
    5ea4:	4b                   	dec    %bx
    5ea5:	4b                   	dec    %bx
    5ea6:	4b                   	dec    %bx
    5ea7:	4b                   	dec    %bx
    5ea8:	4b                   	dec    %bx
    5ea9:	4b                   	dec    %bx
    5eaa:	4b                   	dec    %bx
    5eab:	4b                   	dec    %bx
    5eac:	4b                   	dec    %bx
    5ead:	4b                   	dec    %bx
    5eae:	4b                   	dec    %bx
    5eaf:	4b                   	dec    %bx
    5eb0:	4b                   	dec    %bx
    5eb1:	4b                   	dec    %bx
    5eb2:	4b                   	dec    %bx
    5eb3:	4b                   	dec    %bx
    5eb4:	4b                   	dec    %bx
    5eb5:	4b                   	dec    %bx
    5eb6:	4b                   	dec    %bx
    5eb7:	4b                   	dec    %bx
    5eb8:	4b                   	dec    %bx
    5eb9:	4b                   	dec    %bx
    5eba:	4b                   	dec    %bx
    5ebb:	4b                   	dec    %bx
    5ebc:	4b                   	dec    %bx
    5ebd:	4b                   	dec    %bx
    5ebe:	4b                   	dec    %bx
    5ebf:	4b                   	dec    %bx
    5ec0:	4b                   	dec    %bx
    5ec1:	4b                   	dec    %bx
    5ec2:	4b                   	dec    %bx
    5ec3:	4b                   	dec    %bx
    5ec4:	4b                   	dec    %bx
    5ec5:	4b                   	dec    %bx
    5ec6:	4b                   	dec    %bx
    5ec7:	4b                   	dec    %bx
    5ec8:	4b                   	dec    %bx
    5ec9:	4b                   	dec    %bx
    5eca:	4b                   	dec    %bx
    5ecb:	4b                   	dec    %bx
    5ecc:	4b                   	dec    %bx
    5ecd:	4b                   	dec    %bx
    5ece:	4b                   	dec    %bx
    5ecf:	4b                   	dec    %bx
    5ed0:	4b                   	dec    %bx
    5ed1:	4b                   	dec    %bx
    5ed2:	4b                   	dec    %bx
    5ed3:	4b                   	dec    %bx
    5ed4:	4b                   	dec    %bx
    5ed5:	4b                   	dec    %bx
    5ed6:	4b                   	dec    %bx
    5ed7:	4b                   	dec    %bx
    5ed8:	4b                   	dec    %bx
    5ed9:	4b                   	dec    %bx
    5eda:	4b                   	dec    %bx
    5edb:	4b                   	dec    %bx
    5edc:	4b                   	dec    %bx
    5edd:	4b                   	dec    %bx
    5ede:	4b                   	dec    %bx
    5edf:	4b                   	dec    %bx
    5ee0:	4b                   	dec    %bx
    5ee1:	4b                   	dec    %bx
    5ee2:	4b                   	dec    %bx
    5ee3:	4b                   	dec    %bx
    5ee4:	4b                   	dec    %bx
    5ee5:	4b                   	dec    %bx
    5ee6:	4b                   	dec    %bx
    5ee7:	4b                   	dec    %bx
    5ee8:	4b                   	dec    %bx
    5ee9:	4b                   	dec    %bx
    5eea:	4b                   	dec    %bx
    5eeb:	4b                   	dec    %bx
    5eec:	4b                   	dec    %bx
    5eed:	4b                   	dec    %bx
    5eee:	4b                   	dec    %bx
    5eef:	4b                   	dec    %bx
    5ef0:	4b                   	dec    %bx
    5ef1:	4b                   	dec    %bx
    5ef2:	4b                   	dec    %bx
    5ef3:	4b                   	dec    %bx
    5ef4:	4b                   	dec    %bx
    5ef5:	4b                   	dec    %bx
    5ef6:	4b                   	dec    %bx
    5ef7:	4b                   	dec    %bx
    5ef8:	4b                   	dec    %bx
    5ef9:	4b                   	dec    %bx
    5efa:	4b                   	dec    %bx
    5efb:	4b                   	dec    %bx
    5efc:	4b                   	dec    %bx
    5efd:	4b                   	dec    %bx
    5efe:	4b                   	dec    %bx
    5eff:	4b                   	dec    %bx
    5f00:	4b                   	dec    %bx
    5f01:	4b                   	dec    %bx
    5f02:	4b                   	dec    %bx
    5f03:	4b                   	dec    %bx
    5f04:	4b                   	dec    %bx
    5f05:	4b                   	dec    %bx
    5f06:	4b                   	dec    %bx
    5f07:	4b                   	dec    %bx
    5f08:	4b                   	dec    %bx
    5f09:	4b                   	dec    %bx
    5f0a:	4b                   	dec    %bx
    5f0b:	4b                   	dec    %bx
    5f0c:	4b                   	dec    %bx
    5f0d:	4b                   	dec    %bx
    5f0e:	4b                   	dec    %bx
    5f0f:	4b                   	dec    %bx
    5f10:	4b                   	dec    %bx
    5f11:	4b                   	dec    %bx
    5f12:	4b                   	dec    %bx
    5f13:	4b                   	dec    %bx
    5f14:	4b                   	dec    %bx
    5f15:	4b                   	dec    %bx
    5f16:	4b                   	dec    %bx
    5f17:	4b                   	dec    %bx
    5f18:	4b                   	dec    %bx
    5f19:	4b                   	dec    %bx
    5f1a:	4b                   	dec    %bx
    5f1b:	4b                   	dec    %bx
    5f1c:	4b                   	dec    %bx
    5f1d:	4b                   	dec    %bx
    5f1e:	4b                   	dec    %bx
    5f1f:	4b                   	dec    %bx
    5f20:	4b                   	dec    %bx
    5f21:	4b                   	dec    %bx
    5f22:	4b                   	dec    %bx
    5f23:	4b                   	dec    %bx
    5f24:	4b                   	dec    %bx
    5f25:	4b                   	dec    %bx
    5f26:	4b                   	dec    %bx
    5f27:	4b                   	dec    %bx
    5f28:	4b                   	dec    %bx
    5f29:	4b                   	dec    %bx
    5f2a:	4b                   	dec    %bx
    5f2b:	4b                   	dec    %bx
    5f2c:	4b                   	dec    %bx
    5f2d:	4b                   	dec    %bx
    5f2e:	4b                   	dec    %bx
    5f2f:	4b                   	dec    %bx
    5f30:	4b                   	dec    %bx
    5f31:	4b                   	dec    %bx
    5f32:	4b                   	dec    %bx
    5f33:	4b                   	dec    %bx
    5f34:	4b                   	dec    %bx
    5f35:	4b                   	dec    %bx
    5f36:	4b                   	dec    %bx
    5f37:	4b                   	dec    %bx
    5f38:	4b                   	dec    %bx
    5f39:	4b                   	dec    %bx
    5f3a:	4b                   	dec    %bx
    5f3b:	4b                   	dec    %bx
    5f3c:	4b                   	dec    %bx
    5f3d:	4b                   	dec    %bx
    5f3e:	4b                   	dec    %bx
    5f3f:	4b                   	dec    %bx
    5f40:	4b                   	dec    %bx
    5f41:	4b                   	dec    %bx
    5f42:	4b                   	dec    %bx
    5f43:	4b                   	dec    %bx
    5f44:	4b                   	dec    %bx
    5f45:	4b                   	dec    %bx
    5f46:	4b                   	dec    %bx
    5f47:	4b                   	dec    %bx
    5f48:	4b                   	dec    %bx
    5f49:	4b                   	dec    %bx
    5f4a:	4b                   	dec    %bx
    5f4b:	4b                   	dec    %bx
    5f4c:	4b                   	dec    %bx
    5f4d:	4b                   	dec    %bx
    5f4e:	4b                   	dec    %bx
    5f4f:	4b                   	dec    %bx
    5f50:	4b                   	dec    %bx
    5f51:	4b                   	dec    %bx
    5f52:	4b                   	dec    %bx
    5f53:	4b                   	dec    %bx
    5f54:	4b                   	dec    %bx
    5f55:	4b                   	dec    %bx
    5f56:	4b                   	dec    %bx
    5f57:	4b                   	dec    %bx
    5f58:	4b                   	dec    %bx
    5f59:	4b                   	dec    %bx
    5f5a:	4b                   	dec    %bx
    5f5b:	4b                   	dec    %bx
    5f5c:	4b                   	dec    %bx
    5f5d:	4b                   	dec    %bx
    5f5e:	4b                   	dec    %bx
    5f5f:	4b                   	dec    %bx
    5f60:	4b                   	dec    %bx
    5f61:	4b                   	dec    %bx
    5f62:	4b                   	dec    %bx
    5f63:	4b                   	dec    %bx
    5f64:	4b                   	dec    %bx
    5f65:	4b                   	dec    %bx
    5f66:	4b                   	dec    %bx
    5f67:	4b                   	dec    %bx
    5f68:	4b                   	dec    %bx
    5f69:	4b                   	dec    %bx
    5f6a:	4b                   	dec    %bx
    5f6b:	4b                   	dec    %bx
    5f6c:	4b                   	dec    %bx
    5f6d:	4b                   	dec    %bx
    5f6e:	4b                   	dec    %bx
    5f6f:	4b                   	dec    %bx
    5f70:	4b                   	dec    %bx
    5f71:	4b                   	dec    %bx
    5f72:	4b                   	dec    %bx
    5f73:	4b                   	dec    %bx
    5f74:	4b                   	dec    %bx
    5f75:	4b                   	dec    %bx
    5f76:	4b                   	dec    %bx
    5f77:	4b                   	dec    %bx
    5f78:	4b                   	dec    %bx
    5f79:	4b                   	dec    %bx
    5f7a:	4b                   	dec    %bx
    5f7b:	4b                   	dec    %bx
    5f7c:	4b                   	dec    %bx
    5f7d:	4b                   	dec    %bx
    5f7e:	4b                   	dec    %bx
    5f7f:	4b                   	dec    %bx
    5f80:	4b                   	dec    %bx
    5f81:	4b                   	dec    %bx
    5f82:	4b                   	dec    %bx
    5f83:	4b                   	dec    %bx
    5f84:	4b                   	dec    %bx
    5f85:	4b                   	dec    %bx
    5f86:	4b                   	dec    %bx
    5f87:	4b                   	dec    %bx
    5f88:	4b                   	dec    %bx
    5f89:	4b                   	dec    %bx
    5f8a:	4b                   	dec    %bx
    5f8b:	4b                   	dec    %bx
    5f8c:	4b                   	dec    %bx
    5f8d:	4b                   	dec    %bx
    5f8e:	4b                   	dec    %bx
    5f8f:	4b                   	dec    %bx
    5f90:	4b                   	dec    %bx
    5f91:	4b                   	dec    %bx
    5f92:	4b                   	dec    %bx
    5f93:	4b                   	dec    %bx
    5f94:	4b                   	dec    %bx
    5f95:	4b                   	dec    %bx
    5f96:	4b                   	dec    %bx
    5f97:	4b                   	dec    %bx
    5f98:	4b                   	dec    %bx
    5f99:	4b                   	dec    %bx
    5f9a:	4b                   	dec    %bx
    5f9b:	4b                   	dec    %bx
    5f9c:	4b                   	dec    %bx
    5f9d:	4b                   	dec    %bx
    5f9e:	4b                   	dec    %bx
    5f9f:	4b                   	dec    %bx
    5fa0:	4b                   	dec    %bx
    5fa1:	4b                   	dec    %bx
    5fa2:	4b                   	dec    %bx
    5fa3:	4b                   	dec    %bx
    5fa4:	4b                   	dec    %bx
    5fa5:	4b                   	dec    %bx
    5fa6:	4b                   	dec    %bx
    5fa7:	4b                   	dec    %bx
    5fa8:	4b                   	dec    %bx
    5fa9:	4b                   	dec    %bx
    5faa:	4b                   	dec    %bx
    5fab:	4b                   	dec    %bx
    5fac:	4b                   	dec    %bx
    5fad:	4b                   	dec    %bx
    5fae:	4b                   	dec    %bx
    5faf:	4b                   	dec    %bx
    5fb0:	4b                   	dec    %bx
    5fb1:	4b                   	dec    %bx
    5fb2:	4b                   	dec    %bx
    5fb3:	4b                   	dec    %bx
    5fb4:	4b                   	dec    %bx
    5fb5:	4b                   	dec    %bx
    5fb6:	4b                   	dec    %bx
    5fb7:	4b                   	dec    %bx
    5fb8:	4b                   	dec    %bx
    5fb9:	4b                   	dec    %bx
    5fba:	4b                   	dec    %bx
    5fbb:	4b                   	dec    %bx
    5fbc:	4b                   	dec    %bx
    5fbd:	4b                   	dec    %bx
    5fbe:	4b                   	dec    %bx
    5fbf:	4b                   	dec    %bx
    5fc0:	4b                   	dec    %bx
    5fc1:	4b                   	dec    %bx
    5fc2:	4b                   	dec    %bx
    5fc3:	4b                   	dec    %bx
    5fc4:	4b                   	dec    %bx
    5fc5:	4b                   	dec    %bx
    5fc6:	4b                   	dec    %bx
    5fc7:	4b                   	dec    %bx
    5fc8:	4b                   	dec    %bx
    5fc9:	4b                   	dec    %bx
    5fca:	4b                   	dec    %bx
    5fcb:	4b                   	dec    %bx
    5fcc:	4b                   	dec    %bx
    5fcd:	4b                   	dec    %bx
    5fce:	4b                   	dec    %bx
    5fcf:	4b                   	dec    %bx
    5fd0:	4b                   	dec    %bx
    5fd1:	4b                   	dec    %bx
    5fd2:	4b                   	dec    %bx
    5fd3:	4b                   	dec    %bx
    5fd4:	4b                   	dec    %bx
    5fd5:	4b                   	dec    %bx
    5fd6:	4b                   	dec    %bx
    5fd7:	4b                   	dec    %bx
    5fd8:	4b                   	dec    %bx
    5fd9:	4b                   	dec    %bx
    5fda:	4b                   	dec    %bx
    5fdb:	4b                   	dec    %bx
    5fdc:	4b                   	dec    %bx
    5fdd:	4b                   	dec    %bx
    5fde:	4b                   	dec    %bx
    5fdf:	4b                   	dec    %bx
    5fe0:	4b                   	dec    %bx
    5fe1:	4b                   	dec    %bx
    5fe2:	4b                   	dec    %bx
    5fe3:	4b                   	dec    %bx
    5fe4:	4b                   	dec    %bx
    5fe5:	4b                   	dec    %bx
    5fe6:	4b                   	dec    %bx
    5fe7:	4b                   	dec    %bx
    5fe8:	4b                   	dec    %bx
    5fe9:	4b                   	dec    %bx
    5fea:	4b                   	dec    %bx
    5feb:	4b                   	dec    %bx
    5fec:	4b                   	dec    %bx
    5fed:	4b                   	dec    %bx
    5fee:	4b                   	dec    %bx
    5fef:	4b                   	dec    %bx
    5ff0:	4b                   	dec    %bx
    5ff1:	4b                   	dec    %bx
    5ff2:	4b                   	dec    %bx
    5ff3:	4b                   	dec    %bx
    5ff4:	4b                   	dec    %bx
    5ff5:	4b                   	dec    %bx
    5ff6:	4b                   	dec    %bx
    5ff7:	4b                   	dec    %bx
    5ff8:	4b                   	dec    %bx
    5ff9:	4b                   	dec    %bx
    5ffa:	4b                   	dec    %bx
    5ffb:	4b                   	dec    %bx
    5ffc:	4b                   	dec    %bx
    5ffd:	4b                   	dec    %bx
    5ffe:	4b                   	dec    %bx
    5fff:	4b                   	dec    %bx
    6000:	4b                   	dec    %bx
    6001:	4b                   	dec    %bx
    6002:	4b                   	dec    %bx
    6003:	4b                   	dec    %bx
    6004:	4b                   	dec    %bx
    6005:	4b                   	dec    %bx
    6006:	4b                   	dec    %bx
    6007:	4b                   	dec    %bx
    6008:	4b                   	dec    %bx
    6009:	4b                   	dec    %bx
    600a:	4b                   	dec    %bx
    600b:	4b                   	dec    %bx
    600c:	4b                   	dec    %bx
    600d:	4b                   	dec    %bx
    600e:	4b                   	dec    %bx
    600f:	4b                   	dec    %bx
    6010:	4b                   	dec    %bx
    6011:	4b                   	dec    %bx
    6012:	4b                   	dec    %bx
    6013:	4b                   	dec    %bx
    6014:	4b                   	dec    %bx
    6015:	4b                   	dec    %bx
    6016:	4b                   	dec    %bx
    6017:	4b                   	dec    %bx
    6018:	4b                   	dec    %bx
    6019:	4b                   	dec    %bx
    601a:	4b                   	dec    %bx
    601b:	4b                   	dec    %bx
    601c:	4b                   	dec    %bx
    601d:	4b                   	dec    %bx
    601e:	4b                   	dec    %bx
    601f:	4b                   	dec    %bx
    6020:	4b                   	dec    %bx
    6021:	4b                   	dec    %bx
    6022:	4b                   	dec    %bx
    6023:	4b                   	dec    %bx
    6024:	4b                   	dec    %bx
    6025:	4b                   	dec    %bx
    6026:	4b                   	dec    %bx
    6027:	4b                   	dec    %bx
    6028:	4b                   	dec    %bx
    6029:	4b                   	dec    %bx
    602a:	4b                   	dec    %bx
    602b:	4b                   	dec    %bx
    602c:	4b                   	dec    %bx
    602d:	4b                   	dec    %bx
    602e:	4b                   	dec    %bx
    602f:	4b                   	dec    %bx
    6030:	4b                   	dec    %bx
    6031:	4b                   	dec    %bx
    6032:	4b                   	dec    %bx
    6033:	4b                   	dec    %bx
    6034:	4b                   	dec    %bx
    6035:	4b                   	dec    %bx
    6036:	4b                   	dec    %bx
    6037:	4b                   	dec    %bx
    6038:	4b                   	dec    %bx
    6039:	4b                   	dec    %bx
    603a:	4b                   	dec    %bx
    603b:	4b                   	dec    %bx
    603c:	4b                   	dec    %bx
    603d:	4b                   	dec    %bx
    603e:	4b                   	dec    %bx
    603f:	34 00                	xor    $0x0,%al
    6041:	00 00                	add    %al,(%bx,%si)
    6043:	00 f0                	add    %dh,%al
	...
    6175:	90                   	nop
	...
    617e:	8b 1d                	mov    (%di),%bx
    6180:	3c 09                	cmp    $0x9,%al
    6182:	00 00                	add    %al,(%bx,%si)
    6184:	b1 24                	mov    $0x24,%cl
    6186:	66 b8 05 25 cd 21    	mov    $0x21cd2505,%eax
    618c:	8b 1d                	mov    (%di),%bx
    618e:	40                   	inc    %ax
    618f:	09 00                	or     %ax,(%bx,%si)
    6191:	00 b1 23 66          	add    %dh,0x6623(%bx,%di)
    6195:	b8 05 25             	mov    $0x2505,%ax
    6198:	cd 21                	int    $0x21
    619a:	c3                   	ret
    619b:	b1 24                	mov    $0x24,%cl
    619d:	66 b8 03 25 cd 21    	mov    $0x21cd2503,%eax
    61a3:	89 1d                	mov    %bx,(%di)
    61a5:	3c 09                	cmp    $0x9,%al
    61a7:	00 00                	add    %al,(%bx,%si)
    61a9:	b1 23                	mov    $0x23,%cl
    61ab:	66 b8 03 25 cd 21    	mov    $0x21cd2503,%eax
    61b1:	89 1d                	mov    %bx,(%di)
    61b3:	40                   	inc    %ax
    61b4:	09 00                	or     %ax,(%bx,%si)
    61b6:	00 8b 1d 2b          	add    %cl,0x2b1d(%bp,%di)
    61ba:	09 00                	or     %ax,(%bx,%si)
    61bc:	00 c1                	add    %al,%cl
    61be:	e3 10                	jcxz   0x61d0
    61c0:	66 bb 06 00 b1 23    	mov    $0x23b10006,%ebx
    61c6:	66 b8 05 25 cd 21    	mov    $0x21cd2505,%eax
    61cc:	8b 1d                	mov    (%di),%bx
    61ce:	2b 09                	sub    (%bx,%di),%cx
    61d0:	00 00                	add    %al,(%bx,%si)
    61d2:	c1 e3 10             	shl    $0x10,%bx
    61d5:	66 bb 03 00 b1 24    	mov    $0x24b10003,%ebx
    61db:	66 b8 05 25 cd 21    	mov    $0x21cd2505,%eax
    61e1:	c3                   	ret
    61e2:	fa                   	cli
    61e3:	8c d8                	mov    %ds,%ax
    61e5:	a3 33 09             	mov    %ax,0x933
    61e8:	00 00                	add    %al,(%bx,%si)
    61ea:	8e d0                	mov    %ax,%ss
    61ec:	bc 05 08             	mov    $0x805,%sp
    61ef:	00 00                	add    %al,(%bx,%si)
    61f1:	33 c0                	xor    %ax,%ax
    61f3:	8e e8                	mov    %ax,%gs
    61f5:	8e e0                	mov    %ax,%fs
    61f7:	66 b8 0b 35 cd 21    	mov    $0x21cd350b,%eax
    61fd:	25 00 e0             	and    $0xe000,%ax
    6200:	ff                   	(bad)
    6201:	ff 8b d8 c1          	decw   -0x3e28(%bp,%di)
    6205:	eb 09                	jmp    0x6210
    6207:	2b c3                	sub    %bx,%ax
    6209:	0f 82 9b 01          	jb     0x63a8
    620d:	00 00                	add    %al,(%bx,%si)
    620f:	2d 00 00             	sub    $0x0,%ax
    6212:	0a 00                	or     (%bx,%si),%al
    6214:	3d 00 00             	cmp    $0x0,%ax
    6217:	00 10                	add    %dl,(%bx,%si)
    6219:	76 05                	jbe    0x6220
    621b:	b8 00 00             	mov    $0x0,%ax
    621e:	00 10                	add    %dl,(%bx,%si)
    6220:	a3 13 09             	mov    %ax,0x913
    6223:	00 00                	add    %al,(%bx,%si)
    6225:	66 b8 0c 35 8b 0d    	mov    $0xd8b350c,%eax
    622b:	13 09                	adc    (%bx,%di),%cx
    622d:	00 00                	add    %al,(%bx,%si)
    622f:	cd 21                	int    $0x21
    6231:	0f 82 7e 01          	jb     0x63b3
    6235:	00 00                	add    %al,(%bx,%si)
    6237:	a3 0b 09             	mov    %ax,0x90b
    623a:	00 00                	add    %al,(%bx,%si)
    623c:	03 05                	add    (%di),%ax
    623e:	13 09                	adc    (%bx,%di),%cx
    6240:	00 00                	add    %al,(%bx,%si)
    6242:	a3 0f 09             	mov    %ax,0x90f
    6245:	00 00                	add    %al,(%bx,%si)
    6247:	33 c9                	xor    %cx,%cx
    6249:	ba 03 bf             	mov    $0xbf03,%dx
    624c:	00 00                	add    %al,(%bx,%si)
    624e:	66 bb 01 05 66 b8    	mov    $0xb8660501,%ebx
    6254:	2b 25                	sub    (%di),%sp
    6256:	cd 21                	int    $0x21
    6258:	0f 82 4c 01          	jb     0x63a8
    625c:	00 00                	add    %al,(%bx,%si)
    625e:	b4 62                	mov    $0x62,%ah
    6260:	cd 21                	int    $0x21
    6262:	0f b7 db             	movzww %bx,%bx
    6265:	c1 e3 04             	shl    $0x4,%bx
    6268:	03 1d                	add    (%di),%bx
    626a:	07                   	pop    %es
    626b:	08 00                	or     %al,(%bx,%si)
    626d:	00 89 1d 1f          	add    %cl,0x1f1d(%bx,%di)
    6271:	09 00                	or     %ax,(%bx,%si)
    6273:	00 b8 00 00          	add    %bh,0x0(%bx,%si)
    6277:	0a 00                	or     (%bx,%si),%al
    6279:	03 05                	add    (%di),%ax
    627b:	07                   	pop    %es
    627c:	08 00                	or     %al,(%bx,%si)
    627e:	00 a3 17 09          	add    %ah,0x917(%bp,%di)
    6282:	00 00                	add    %al,(%bx,%si)
    6284:	b8 00 80             	mov    $0x8000,%ax
    6287:	0b 00                	or     (%bx,%si),%ax
    6289:	03 05                	add    (%di),%ax
    628b:	07                   	pop    %es
    628c:	08 00                	or     %al,(%bx,%si)
    628e:	00 a3 1b 09          	add    %ah,0x91b(%bp,%di)
    6292:	00 00                	add    %al,(%bx,%si)
    6294:	b8 00 00             	mov    $0x0,%ax
    6297:	0b 00                	or     (%bx,%si),%ax
    6299:	03 05                	add    (%di),%ax
    629b:	07                   	pop    %es
    629c:	08 00                	or     %al,(%bx,%si)
    629e:	00 a3 23 09          	add    %ah,0x923(%bp,%di)
    62a2:	00 00                	add    %al,(%bx,%si)
    62a4:	66 b8 09 25 cd 21    	mov    $0x21cd2509,%eax
    62aa:	0f b7 db             	movzww %bx,%bx
    62ad:	89 1d                	mov    %bx,(%di)
    62af:	2b 09                	sub    (%bx,%di),%cx
    62b1:	00 00                	add    %al,(%bx,%si)
    62b3:	c1 e3 04             	shl    $0x4,%bx
    62b6:	03 1d                	add    (%di),%bx
    62b8:	07                   	pop    %es
    62b9:	08 00                	or     %al,(%bx,%si)
    62bb:	00 89 1d 2f          	add    %cl,0x2f1d(%bx,%di)
    62bf:	09 00                	or     %ax,(%bx,%si)
    62c1:	00 ba 0b 08          	add    %bh,0x80b(%bp,%si)
    62c5:	00 00                	add    %al,(%bx,%si)
    62c7:	89 15                	mov    %dx,(%di)
    62c9:	27                   	daa
    62ca:	09 00                	or     %ax,(%bx,%si)
    62cc:	00 b4 1a cd          	add    %dh,-0x32e6(%si)
    62d0:	21 e8                	and    %bp,%ax
    62d2:	c5 fe ff             	(bad)
    62d5:	ff                   	(bad)
    62d6:	fb                   	sti
    62d7:	eb 39                	jmp    0x6312
    62d9:	90                   	nop
    62da:	24 24                	and    $0x24,%al
    62dc:	24 24                	and    $0x24,%al
    62de:	24 24                	and    $0x24,%al
    62e0:	24 24                	and    $0x24,%al
    62e2:	9d                   	popf
    62e3:	5c                   	pop    %sp
    62e4:	8c 08                	mov    %cs,(%bx,%si)
    62e6:	b4 0a                	mov    $0xa,%ah
    62e8:	00 00                	add    %al,(%bx,%si)
    62ea:	03 bf 00 00          	add    0x0(%bx),%di
    62ee:	a0 ed 09             	mov    0x9ed,%al
    62f1:	d5 be                	aad    $0xbe
    62f3:	a4                   	movsb  %ds:(%si),%es:(%di)
    62f4:	ab                   	stos   %ax,%es:(%di)
    62f5:	16                   	push   %ss
    62f6:	d0 9b 00 b7          	rcrb   $1,-0x4900(%bp,%di)
    62fa:	de ba be 67          	fidivrs 0x67be(%bp,%si)
    62fe:	c5 8f 00 b7          	lds    -0x4900(%bx),%cx
    6302:	de ba be 67          	fidivrs 0x67be(%bp,%si)
    6306:	c5 8f 00 b7          	lds    -0x4900(%bx),%cx
    630a:	de ba be 67          	fidivrs 0x67be(%bp,%si)
    630e:	20 0c                	and    %cl,(%si)
    6310:	00 00                	add    %al,(%bx,%si)
    6312:	be b4 0a             	mov    $0xab4,%si
    6315:	00 00                	add    %al,(%bx,%si)
    6317:	b9 03 bf             	mov    $0xbf03,%cx
    631a:	00 00                	add    %al,(%bx,%si)
    631c:	2b ce                	sub    %si,%cx
    631e:	c1 e9 03             	shr    $0x3,%cx
    6321:	33 c0                	xor    %ax,%ax
    6323:	33 06 d1 c8          	xor    -0x372f,%ax
    6327:	33 46 04             	xor    0x4(%bp),%ax
    632a:	d1 c8                	ror    $1,%ax
    632c:	83 c6 08             	add    $0x8,%si
    632f:	e2 f2                	loop   0x6323
    6331:	8b 1d                	mov    (%di),%bx
    6333:	a8 0a                	test   $0xa,%al
    6335:	00 00                	add    %al,(%bx,%si)
    6337:	3b c3                	cmp    %bx,%ax
    6339:	74 21                	je     0x635c
    633b:	81 fb ad de          	cmp    $0xdead,%bx
    633f:	34 12                	xor    $0x12,%al
    6341:	74 0b                	je     0x634e
    6343:	b4 09                	mov    $0x9,%ah
    6345:	ba 00 0c             	mov    $0xc00,%dx
    6348:	00 00                	add    %al,(%bx,%si)
    634a:	cd 21                	int    $0x21
    634c:	eb 50                	jmp    0x639e
    634e:	ba f2 0b             	mov    $0xbf2,%dx
    6351:	00 00                	add    %al,(%bx,%si)
    6353:	b4 09                	mov    $0x9,%ah
    6355:	cd 21                	int    $0x21
    6357:	e9 7b 02             	jmp    0x65d5
    635a:	00 00                	add    %al,(%bx,%si)
    635c:	b9 04 00             	mov    $0x4,%cx
    635f:	00 00                	add    %al,(%bx,%si)
    6361:	be b4 0a             	mov    $0xab4,%si
    6364:	00 00                	add    %al,(%bx,%si)
    6366:	b8 e1 ab             	mov    $0xabe1,%ax
    6369:	24 93                	and    $0x93,%al
    636b:	bb fa 9e             	mov    $0x9efa,%bx
    636e:	9a 43 31 06 31       	lcall  $0x3106,$0x3143
    6373:	5e                   	pop    %si
    6374:	04 83                	add    $0x83,%al
    6376:	c6                   	(bad)
    6377:	08 e2                	or     %ah,%dl
    6379:	f6 be 20 0c          	idivb  0xc20(%bp)
    637d:	00 00                	add    %al,(%bx,%si)
    637f:	b9 03 bf             	mov    $0xbf03,%cx
    6382:	00 00                	add    %al,(%bx,%si)
    6384:	2b ce                	sub    %si,%cx
    6386:	c1 e9 03             	shr    $0x3,%cx
    6389:	31 06 31 5e          	xor    %ax,0x5e31
    638d:	04 d1                	add    $0xd1,%al
    638f:	cb                   	lret
    6390:	d1 c8                	ror    $1,%ax
    6392:	33 c1                	xor    %cx,%ax
    6394:	83 c6 08             	add    $0x8,%si
    6397:	e2 f0                	loop   0x6389
    6399:	e9 39 02             	jmp    0x65d5
    639c:	00 00                	add    %al,(%bx,%si)
    639e:	fa                   	cli
    639f:	e8 da fd             	call   0x617c
    63a2:	ff                   	(bad)
    63a3:	ff 66 b8             	jmp    *-0x48(%bp)
    63a6:	00 4c cd             	add    %cl,-0x33(%si)
    63a9:	21 ba 91 0b          	and    %di,0xb91(%bp,%si)
    63ad:	00 00                	add    %al,(%bx,%si)
    63af:	b4 09                	mov    $0x9,%ah
    63b1:	cd 21                	int    $0x21
    63b3:	eb e9                	jmp    0x639e
    63b5:	ba ac 0b             	mov    $0xbac,%dx
    63b8:	00 00                	add    %al,(%bx,%si)
    63ba:	b4 09                	mov    $0x9,%ah
    63bc:	cd 21                	int    $0x21
    63be:	eb de                	jmp    0x639e
    63c0:	ba d0 0b             	mov    $0xbd0,%dx
    63c3:	00 00                	add    %al,(%bx,%si)
    63c5:	b4 09                	mov    $0x9,%ah
    63c7:	cd 21                	int    $0x21
    63c9:	eb d3                	jmp    0x639e
    63cb:	45                   	inc    %bp
    63cc:	58                   	pop    %ax
    63cd:	54                   	push   %sp
    63ce:	45                   	inc    %bp
    63cf:	4e                   	dec    %si
    63d0:	44                   	inc    %sp
    63d1:	45                   	inc    %bp
    63d2:	52                   	push   %dx
    63d3:	3a 20                	cmp    (%bx,%si),%ah
    63d5:	4d                   	dec    %bp
    63d6:	65 6d                	gs insw (%dx),%es:(%di)
    63d8:	6f                   	outsw  %ds:(%si),(%dx)
    63d9:	72 79                	jb     0x6454
    63db:	20 45 72             	and    %al,0x72(%di)
    63de:	72 6f                	jb     0x644f
    63e0:	72 0d                	jb     0x63ef
    63e2:	0a 0d                	or     (%di),%cl
    63e4:	0a 24                	or     (%si),%ah
    63e6:	45                   	inc    %bp
    63e7:	58                   	pop    %ax
    63e8:	54                   	push   %sp
    63e9:	45                   	inc    %bp
    63ea:	4e                   	dec    %si
    63eb:	44                   	inc    %sp
    63ec:	45                   	inc    %bp
    63ed:	52                   	push   %dx
    63ee:	3a 20                	cmp    (%bx,%si),%ah
    63f0:	4d                   	dec    %bp
    63f1:	65 6d                	gs insw (%dx),%es:(%di)
    63f3:	6f                   	outsw  %ds:(%si),(%dx)
    63f4:	72 79                	jb     0x646f
    63f6:	20 41 6c             	and    %al,0x6c(%bx,%di)
    63f9:	6c                   	insb   (%dx),%es:(%di)
    63fa:	6f                   	outsw  %ds:(%si),(%dx)
    63fb:	63 61 74             	arpl   %sp,0x74(%bx,%di)
    63fe:	65                   	gs
    63ff:	20                   	.byte 0x20

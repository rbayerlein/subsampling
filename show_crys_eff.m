fid = fopen('crys_eff_readIn_fromServer2', 'rb');
%fseek(fid, 3095517*4, 'bof'); 
crys_eff = fread(fid, 672*840, 'float');
crys_eff = reshape(crys_eff, 672, 840);
imshow(crys_eff, []); 
colorbar;
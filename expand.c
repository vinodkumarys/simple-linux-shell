void expand()
{
  int i=0,j=0,k=0,t=0,len = strlen(buf),x=0;
  char ch;
  char tmp[ PATH_MAX + 1],*str;

  bzero(ex_buf,sizeof(ex_buf));

  while(buf[i] != '\0' && i < len)
    {
      ch = buf[i];
      if(ch == '$')
        {
          k = i + 1;
          while((tmp[t++] = buf[k++]) != ' ')
            ;
          tmp[t] = '\0';
          str = getenv(tmp);
          while(str[x] != '\0')
            ex_buf[j++] = str[x++];
          i = k;
        }
      else if(ch == '*')
      {
	  }
      else
        ex_buf[j++] = ch;
      i++;
    }

  ex_buf[j] = '\0';
}

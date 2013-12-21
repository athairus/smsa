" if has("autocmd")
"   filetype plugin indent on
" endif

colorscheme elflord

set showcmd
" set nowrap


set mouse=a

" set nocompatible               " be iMproved
: filetype off                   " required!

set tabstop=4       " The width of a TAB is set to 4.
                    " Still it is a \t. It is just that
                    " Vim will interpret it to be having
                    " a width of 4.

set shiftwidth=4    " Indents will have a width of 4

set softtabstop=4   " Sets the number of columns for a TAB

set expandtab       " Expand TABs to spaces

set rtp+=~/.vim/bundle/vundle/
call vundle#rc()

" let Vundle manage Vundle
" required! 
Bundle 'gmarik/vundle'
Bundle 'terryma/vim-smooth-scroll'

filetype plugin indent on

" smooth scroll maps
map <silent> <c-b> :call smooth_scroll#up(&scroll/3, 16, 1)<CR>
map <silent> <c-f> :call smooth_scroll#down(&scroll/3, 16, 1)<CR>


map <ScrollWheelUp> <c-b>
map <ScrollWheelDown> <c-f>

imap <silent> <c-b> <Esc>:call smooth_scroll#up(&scroll/3, 16, 1)<CR>i
imap <silent> <c-f> <Esc>:call smooth_scroll#down(&scroll/3, 16, 1)<CR>i


imap <ScrollWheelUp> <c-b>
imap <ScrollWheelDown> <c-f>

" astyle!
autocmd BufNewFile,BufRead *.cpp,*.c,*.h,*.hpp set formatprg=astyle\ --style=java\ --indent=spaces=4\ --align-pointer=name\ --indent-switches\ --break-blocks\ --pad-oper\ --pad-paren-in\ --unpad-paren\ --fill-empty-lines\ --break-closing-brackets\ --convert-tabs\ --mode=c

" copy with ctrl+c
vmap <C-C> "+y

